import os
from __main__ import vtk, qt, ctk, slicer
import EditorLib
from EditorLib.EditOptions import HelpButton
from EditorLib.EditOptions import EditOptions
from EditorLib import EditUtil
from EditorLib import LabelEffect

from EditorLib import Effect
from EditorLib import LabelEffectLogic

import logging
from copy import copy, deepcopy
import numpy as np


#
# The Editor Extension itself.
#
# This needs to define the hooks to be come an editor effect.
#

#
# CleverSegEffectOptions - see LabelEffect, EditOptions and Effect for superclasses
#

class CleverSegEffectOptions(EditorLib.LabelEffectOptions):
    """ CleverSegEffect-specfic gui
    """

    def __init__(self, parent=0):
        super(CleverSegEffectOptions, self).__init__(parent)
        # ~ print("Made a CleverSegEffectOptions")

        # save a layout manager, get all 3 slice widgets
        editUtil = EditorLib.EditUtil.EditUtil()
        parameterNode = editUtil.getParameterNode()
        lm = slicer.app.layoutManager()
        self.redSliceWidget = lm.sliceWidget('Red')
        self.yellowSliceWidget = lm.sliceWidget('Yellow')
        self.greenSliceWidget = lm.sliceWidget('Green')
        self.parameterNode = parameterNode
        self.attributes = ('MouseTool')
        self.displayName = 'CleverSegEffect Effect'

    def __del__(self):
        super(CleverSegEffectOptions, self).__del__()

    def create(self):
        super(CleverSegEffectOptions, self).create()
        self.helpLabel = qt.QLabel(
            "Run the CleverSeg segmentation on the current label/seed image.\n Background and foreground seeds will be used as starting points to fill in the rest of the volume.",
            self.frame)
        self.frame.layout().addWidget(self.helpLabel)

        # create a "Start Bot" button
        moduleDir = os.path.dirname(slicer.util.modulePath(self.__module__))
        CleverSegIconPath = os.path.join(moduleDir, 'CleverSegEffect.png')
        cleverSegIcon = qt.QIcon(CleverSegIconPath)

        self.botButton = qt.QPushButton(self.frame)
        self.botButton.setIcon(cleverSegIcon)
        self.frame.layout().addWidget(self.botButton)
        self.botButton.connect('clicked()', self.onStartBot)

        self.locRadFrame = qt.QFrame(self.frame)
        self.locRadFrame.setLayout(qt.QHBoxLayout())
        self.frame.layout().addWidget(self.locRadFrame)
        self.widgets.append(self.locRadFrame)

        # HelpButton(self.frame, "TO USE: \n Start the interactive segmenter and initialize the segmentation with any other editor tool. \n KEYS: \n Press the following keys to interact: \n C: copy label slice \n V: paste label slice \n Q: evolve contour in 2D \n E: evolve contour in 3D \n A: toggle between draw/erase modes" )
        HelpButton(self.frame,
                   "TO USE: \n Start the CleverSeg segmenter and initialize the segmentation with any other editor tool. \n KEYS: \n Press the following keys to interact: \n C: start CleverSeg \n S: toggle between seed image and segmentation result \n R: reset CleverSeg \n")
        self.frame.layout().addStretch(1)  # Add vertical spacer

        if hasattr(slicer.modules, 'CSEditorBot'):
            slicer.util.showStatusMessage(slicer.modules.CSEditorBot.logic.currentMessage)
            self.botButton.text = "Stop CleverSeg Segmenter"
            if self.locRadFrame:
                self.locRadFrame.hide()
        else:
            self.botButton.text = "Start CleverSeg Segmenter"
            if self.locRadFrame:
                self.locRadFrame.show()

    def destroy(self):
        self.currentMessage = ""
        slicer.util.showStatusMessage(self.currentMessage)

        super(CleverSegEffectOptions, self).destroy()

    # note: this method needs to be implemented exactly as-is
    # in each leaf subclass so that "self" in the observer
    # is of the correct type
    def updateParameterNode(self, caller, event):
        node = EditUtil.EditUtil().getParameterNode()
        if node != self.parameterNode:
            if self.parameterNode:
                node.RemoveObserver(self.parameterNodeTag)
            self.parameterNode = node
            self.parameterNodeTag = node.AddObserver(vtk.vtkCommand.ModifiedEvent, self.updateGUIFromMRML)

    def setMRMLDefaults(self):
        super(CleverSegEffectOptions, self).setMRMLDefaults()

    def onStartBot(self):

        """Stop CarreraSlice bot to avoid conflicts"""
        if hasattr(slicer.modules, 'editorBot'):
            slicer.modules.editorBot.stop()
            del (slicer.modules.editorBot)

        """create the bot for background editing"""
        if hasattr(slicer.modules, 'CSEditorBot'):
            slicer.modules.CSEditorBot.stop()
            del (slicer.modules.CSEditorBot)
            if self.botButton:
                self.botButton.text = "Start CleverSeg Segmenter"
                slicer.util.showStatusMessage("CleverSeg: stopped")
            if self.locRadFrame:
                self.locRadFrame.show()
        else:
            CleverSegBot(self)
            slicer.modules.CSEditorBot.logic.emergencyStopFunc = self.botEstop;  # save the function that stops bot, destroys CleverSeg segmenter, if things go wrong
            if self.botButton:
                self.botButton.text = "Stop CleverSeg Segmenter"
                self.currentMessage = "CleverSeg started: go to PaintEffect to edit seed image or press C to do CleverSeg"
                slicer.util.showStatusMessage(self.currentMessage)

            if self.locRadFrame:
                self.locRadFrame.hide()

    def updateGUIFromMRML(self, caller, event):
        self.disconnectWidgets()
        super(CleverSegEffectOptions, self).updateGUIFromMRML(caller, event)
        self.connectWidgets()

    def updateMRMLFromGUI(self):
        if self.updatingGUI:
            return
        disableState = self.parameterNode.GetDisableModifiedEvent()
        self.parameterNode.SetDisableModifiedEvent(1)
        super(CleverSegEffectOptions, self).updateMRMLFromGUI()
        self.parameterNode.SetDisableModifiedEvent(disableState)
        if not disableState:
            self.parameterNode.InvokePendingModifiedEvent()

    def botEstop(self):
        if hasattr(slicer.modules, 'CSEditorBot'):
            slicer.modules.CSEditorBot.stop()
            del (slicer.modules.CSEditorBot)
            if self.botButton:
                self.botButton.text = "Start CleverSeg Segmenter"
            if self.locRadFrame:
                self.locRadFrame.show()


class CleverSegBot(object):  # stays active even when running the other editor effects
    """
  Task to run in the background for this effect.
  Receives a reference to the currently active options
  so it can access tools if needed.
  """

    def __init__(self, options):
        self.editUtil = EditUtil.EditUtil()
        self.sliceWidget = options.tools[0].sliceWidget
        # self.sliceWidget = slicer.app.layoutManager().sliceWidget('Red')
        if hasattr(slicer.modules, 'CSEditorBot'):
            slicer.modules.CSEditorBot.active = False
            del (slicer.modules.CSEditorBot)
        slicer.modules.CSEditorBot = self

        self.redSliceWidget = options.redSliceWidget
        self.greenSliceWidget = options.greenSliceWidget
        self.yellowSliceWidget = options.yellowSliceWidget
        self.start()

    def start(self):
        sliceLogic = self.sliceWidget.sliceLogic()
        self.logic = CleverSegEffectLogic(self.redSliceWidget.sliceLogic())

    def stop(self):
        self.logic.destroy()


#
# CleverSegEffectTool
#

class CleverSegEffectTool(LabelEffect.LabelEffectTool):
    """
    One instance of this will be created per-view when the effect
    is selected.  It is responsible for implementing feedback and
    label map changes in response to user input.
    This class observes the editor parameter node to configure itself
    and queries the current view for background and label volume
    nodes to operate on.
    """

    def __init__(self, sliceWidget):
        super(CleverSegEffectTool, self).__init__(sliceWidget)
        # create a logic instance to do the non-gui work
        # self.logic = CleverSegEffectLogic(self.sliceWidget.sliceLogic())

    def cleanup(self):
        super(CleverSegEffectTool, self).cleanup()

    def processEvent(self, caller=None, event=None):
        """
        handle events from the render window interactor
        """

        # let the superclass deal with the event if it wants to
        if super(CleverSegEffectTool, self).processEvent(caller, event):
            return

        # events from the slice node
        if caller and caller.IsA('vtkMRMLSliceNode'):
            # here you can respond to pan/zoom or other changes
            # to the view
            pass


#
# CleverSegEffectLogic
#
class CleverSegEffectLogic(LabelEffect.LabelEffectLogic):
    """
    This class contains helper methods for a given effect
    type.  It can be instanced as needed by an CleverSegEffectTool
    or CleverSegEffectOptions instance in order to compute intermediate
    results (say, for user feedback) or to implement the final
    segmentation editing operation.  This class is split
    from the CleverSegEffectTool so that the operations can be used
    by other code without the need for a view context.
    """

    def __init__(self, sliceLogic):
        print("Preparing CleverSeg interaction")
        self.attributes = ('MouseTool')
        self.displayName = 'CleverSeg Effect'

        # disconnect all shortcuts that may exist, to allow CleverSeg's to work, reconnect once bot is turned off
        slicer.modules.EditorWidget.removeShortcutKeys()
        self.sliceLogic = sliceLogic
        self.editUtil = EditUtil.EditUtil()

        # initialize CleverSeg
        self.init_fCleverSeg()

        self.CleverSegCreated = False

    def getInvalidInputsMessage(self):
        background = self.editUtil.getBackgroundVolume().GetImageData().GetPointData().GetScalars().GetDataType()
        labelInput = self.editUtil.getLabelVolume().GetImageData().GetPointData().GetScalars().GetDataType()
        return "CleverSeg is attempted with image type '{0}' and labelmap " \
               "type '{1}'. CleverSeg only works robustly with 'short' " \
               "image and labelmap types.".format(
            self.editUtil.getBackgroundVolume().GetImageData().GetScalarTypeAsString(),
            self.editUtil.getLabelVolume().GetImageData().GetScalarTypeAsString())

    def areInputsValid(self):
        background = self.editUtil.getBackgroundVolume().GetImageData().GetPointData().GetScalars().GetDataType()
        labelInput = self.editUtil.getLabelVolume().GetImageData().GetPointData().GetScalars().GetDataType()


        if not (background == vtk.VTK_SHORT and labelInput == vtk.VTK_SHORT):
            return False
        return True

    def init_fCleverSeg(self):
        slicer.util.showStatusMessage("Checking CleverSeg inputs...")

        if not self.areInputsValid():
            logging.warning(self.getInvalidInputsMessage())
            background = self.editUtil.getBackgroundVolume()

            labelInput = self.editUtil.getLabelVolume()
            if not slicer.util.confirmOkCancelDisplay(
                    "Current image type is '{0}' and labelmap type is '{1}'. CleverSeg only works "
                    "reliably with 'short' type.\n\nIf the segmentation result is not satisfactory"
                    ", then cast the image and labelmap to 'short' type (using Cast Scalar Volume "
                    "module) or install CleverSeg extension and use CleverSegEffect editor "
                    "tool.".format(self.editUtil.getBackgroundVolume().GetImageData().GetScalarTypeAsString(),
                                   self.editUtil.getLabelVolume().GetImageData().GetScalarTypeAsString()), windowTitle='Editor'):
                logging.warning('CleverSeg is cancelled by the user')
                return

        self.emergencyStopFunc = None
        self.dialogBox = qt.QMessageBox()  # will display messages to draw users attention if he does anything wrong
        self.dialogBox.setWindowTitle("CleverSeg Error")
        self.dialogBox.setWindowModality(qt.Qt.NonModal)  # will allow user to continue interacting with Slicer

        # TODO: check this claim- might be causing leaks
        # set the image, label nodes (this will not change although the user can
        # alter what is bgrnd/frgrnd in editor)
        # Confused about how info propagates UIarray to UIVol, not the other way, NEEDS AUTO TESTS
        self.labelNode = self.editUtil.getLabelVolume()  # labelLogic.GetVolumeNode()
        self.backgroundNode = self.editUtil.getBackgroundVolume()  # backgroundLogic.GetVolumeNode()

        # perform safety check on right images/labels being selected, #set up images
        # if red slice doesnt have a label or image, go no further
        if type(self.backgroundNode) == type(None) or type(self.labelNode) == type(None):
            self.dialogBox.setText("Either Image (must be Background Image) or Label not set in slice views.")
            self.dialogBox.show()

            if self.emergencyStopFunc:
                self.emergencyStopFunc()
            return

        volumesLogic = slicer.modules.volumes.logic()

        self.labelName = self.labelNode.GetName()  # record name of label so user, cant trick us
        self.imgName = self.backgroundNode.GetName()

        if self.sliceViewMatchEditor(
                self.sliceLogic) == False:  # do nothing, exit function if user has played with images
            if self.emergencyStopFunc:
                self.emergencyStopFunc()
            return

        # CleverSeg shortcuts
        resetCS = qt.QKeySequence(qt.Qt.Key_R)  # reset initialization flag
        runCS = qt.QKeySequence(qt.Qt.Key_C)  # run CleverSeg
        editSeed = qt.QKeySequence(qt.Qt.Key_S)  # edit seed labels

        print " keys for reset init = R, run CS = C, edit seed = S"

        self.qtkeyconnections = []
        self.qtkeydefsCleverSeg = [[resetCS, self.resetCleverSegFlag],
                                   [runCS, self.runCleverSeg],
                                   [editSeed, self.editCleverSegSeed]]

        for keydef in self.qtkeydefsCleverSeg:
            s = qt.QShortcut(keydef[0], slicer.util.mainWindow())  # connect this qt event to mainWindow focus
            # s.setContext(1)
            s.connect('activated()', keydef[1])
            # s.connect('activatedAmbiguously()', keydef[1])
            self.qtkeyconnections.append(s)

        self.CSLabelMod_tag = self.sliceLogic.AddObserver("ModifiedEvent",
                                                          self.CleverSegChangeLabelInput)  # put test listener on the whole window

        # fast balanced growth parameters
        self.bSegmenterInitialized = "no"
        self.bEditCleverSegSeed = True
        self.currentMessage = ""

        seedArray = slicer.util.array(self.labelName)
        self.CleverSegSeedArray = seedArray.copy()
        self.CleverSegSegArray = seedArray.copy()
        self.CleverSegSeedArray[:] = 0
        self.CleverSegSegArray[:] = 0

        import vtkSlicerCleverSegModuleLogicPython

        cleverSegMod = vtkSlicerCleverSegModuleLogicPython.vtkCleverSeg()

        cleverSegMod.SetSourceVol(self.backgroundNode.GetImageData())
        cleverSegMod.SetSeedVol(self.labelNode.GetImageData())
        cleverSegMod.Initialization()
        self.cleverSegMod = cleverSegMod

        self.cleverSegCreated = True  # tracks if completed the initializtion (so can do stop correctly) of KSlice

    def sliceViewMatchEditor(self, sliceLogic):
        # if self.dialogBox==type(None): #something deleted teh dialogBox, this function then breaks, bail
        # if self.emergencyStopFunc:
        # self.emergencyStopFunc()
        # return False

        imgNode = sliceLogic.GetBackgroundLayer().GetVolumeNode()
        labelNode = sliceLogic.GetLabelLayer().GetVolumeNode()

        if type(imgNode) == type(None) or type(labelNode) == type(None):
            self.dialogBox.setText("Either image (must be Background Image) or label not set in slice views.")
            self.dialogBox.show()
            if self.emergencyStopFunc:
                self.emergencyStopFunc()
            return False

        dimImg = self.backgroundNode.GetImageData().GetDimensions()
        dimLab = self.labelNode.GetImageData().GetDimensions()
        # ~ dimImg=imgNode.GetImageData().GetDimensions()
        # ~ dimLab=labelNode.GetImageData().GetDimensions()

        if not (dimImg[0] == dimLab[0] and dimImg[1] == dimLab[1] and dimImg[2] == dimLab[
            2]):  # if sizes dont match up(doing this b/c cant reach HelperBox parameters
            self.dialogBox.setText("Mismatched label to image.")
            self.dialogBox.show()
            if self.emergencyStopFunc:
                self.emergencyStopFunc()
            return False

        if (self.imgName == imgNode.GetName()) and (self.labelName == labelNode.GetName()):
            return True
        else:
            self.dialogBox.setText("Set image to values used for starting the CleverSeg bot or stop bot.")
            self.dialogBox.show()
            if self.emergencyStopFunc:
                self.emergencyStopFunc()
            return False

    def CleverSegChangeLabelInput(self, caller, event):

        if self.sliceViewMatchEditor(self.sliceLogic) == False:
            return  # do nothing, exit function

    # run CleverSeg segmenter for the current master volume and label volume

    def runCleverSeg(self):
        if self.bEditCleverSegSeed == True:

            self.currentMessage = "Running CleverSeg..."
            slicer.util.showStatusMessage(self.currentMessage)
            seedArray = slicer.util.array(self.labelNode.GetName())
            self.CleverSegSeedArray[:] = seedArray[:]

            bCSInitialized = False
            if self.bSegmenterInitialized != "no":
                bCSInitialized = True
            self.cleverSegMod.SetSourceVol(self.backgroundNode.GetImageData())
            self.cleverSegMod.SetSeedVol(self.labelNode.GetImageData())
            self.cleverSegMod.SetInitializationFlag(bCSInitialized)
            self.cleverSegMod.RunCS()
            self.CleverSegSegArray[:] = seedArray[:]

            self.labelNode.GetImageData().Modified()
            self.labelNode.Modified()

            self.bSegmenterInitialized = "yes"
            self.bEditCSSeed = False

            self.currentMessage = "CleverSeg complete (): press 'S' to refine annotation, or 'R' to start over"
            slicer.util.showStatusMessage(self.currentMessage)
        else:
            self.currentMessage = "CleverSeg: go to seed labels first by pressing 'S'"
            slicer.util.showStatusMessage(self.currentMessage)

    # reset CleverSeg segmenter
    def resetCleverSegFlag(self):
        self.bSegmenterInitialized = "no"
        self.bEditCleverSegSeed = True
        self.CleverSegSeedArray[:] = 0
        self.CleverSegSegArray[:] = 0

        seedArray = slicer.util.array(self.labelNode.GetName())
        seedArray[:] = 0

        self.labelNode.GetImageData().Modified()
        self.labelNode.Modified()
        print('reset CleverSeg parameters')
        self.currentMessage = "CleverSeg: Go to PaintEffect to draw annotation and press 'C' to run CleverSeg"
        slicer.util.showStatusMessage(self.currentMessage)

    def editCleverSegSeed(self):

        seedArray = slicer.util.array(self.labelNode.GetName())
        if self.bEditCleverSegSeed == False:
            self.CleverSegSegArray[:] = seedArray[:]
            seedArray[:] = self.CleverSegSeedArray[:]
            self.bEditCleverSegSeed = True
            self.labelNode.GetImageData().Modified()
            self.labelNode.Modified()

            print('show seed image')
            self.currentMessage = "CleverSeg: Press 'S' to show the segmentation result; Or go to PaintEffect (to refine annotation) and press 'C' to run CleverSeg"
            slicer.util.showStatusMessage(self.currentMessage)
        else:
            if self.CleverSegSegArray.any() != 0:

                seedArray[:] = self.CleverSegSegArray[:]
                self.bEditCleverSegSeed = False
                self.labelNode.GetImageData().Modified()
                self.labelNode.Modified()

                print('show segmentation')
                self.currentMessage = "CleverSeg: Press 'S' to edit seeds and run CleverSeg again"
                slicer.util.showStatusMessage(self.currentMessage)
            else:
                print('no segmentation result')
                self.currentMessage = "CleverSeg: no segmentation result available"
                slicer.util.showStatusMessage(self.currentMessage)

    def destroy(self):

        # destroy CleverSeg key shortcuts
        for i in range(len(
                self.qtkeydefsCleverSeg)):  # this will be an empty list if the KSlice part has been reached (all CleverSeg functionality disabled)
            keyfun = self.qtkeydefsCleverSeg[i]
            keydef = self.qtkeyconnections[i]
            test1 = keydef.disconnect('activated()', keyfun[1])
            test2 = keydef.disconnect('activatedAmbiguously()', keyfun[1])
            # self.qtkeyconnections.remove(keydef) #remove from list
            keydef.setParent(None)
            # why is this necessary for full disconnect (if removed, get the error that more and more keypresses are required if module is repetedly erased and created
            keydef.delete()  # this causes errors

        # destroy CleverSeg objects
        self.CleverSegSeedArray = None
        self.CleverSegSegArray = None
        self.cleverSegMod = None
        self.currentMessage = ""
        self.imgName = None
        self.labelName = None
        self.labelNode = None
        self.backgroundNode = None

        # remove CleverSeg observer
        self.sliceLogic.RemoveObserver(self.CSLabelMod_tag)

        # put back the editor shortcuts we removed
        slicer.modules.EditorWidget.installShortcutKeys()

        print("Deletion completed")


#
# The CleverSegEffect class definition
#

class CleverSegEffectExtension(LabelEffect.LabelEffect):
    """Organizes the Options, Tool, and Logic classes into a single instance
    that can be managed by the EditBox
    """

    def __init__(self):
        # name is used to define the name of the icon image resource (e.g. CleverSegEffect.png)
        self.name = "CleverSeg"
        # tool tip is displayed on mouse hover
        self.toolTip = "Paint: circular paint brush for label map editing"

        self.options = CleverSegEffectOptions
        self.tool = CleverSegEffectTool
        self.logic = CleverSegEffectLogic


""" Test:

sw = slicer.app.layoutManager().sliceWidget('Red')
import EditorLib
pet = EditorLib.CleverSegEffectTool(sw)

"""


#
# CleverSegEffect
#

class CleverSegEffect:
    """
    This class is the 'hook' for slicer to detect and recognize the extension
    as a loadable scripted module
    """

    def __init__(self, parent):
        parent.dependencies = ["Editor"]
        parent.title = "Editor CleverSeg Effect"
        parent.categories = ["Developer Tools.Editor Extensions"]
        parent.contributors = ["Jonathan Ramos (University of Sao Paulo)"]  # insert your name in the list
        parent.helpText = """Interactive segmentation editor extension."""
        parent.acknowledgementText = """ This editor extension was developed by Jonathan Ramos (USP) """

        # TODO:
        # don't show this module - it only appears in the Editor module
        # parent.hidden = True

        # Add this extension to the editor's list for discovery when the module
        # is created.  Since this module may be discovered before the Editor itself,
        # create the list if it doesn't already exist.
        try:
            slicer.modules.editorExtensions
        except AttributeError:
            slicer.modules.editorExtensions = {}
        slicer.modules.editorExtensions['CleverSegEffect'] = CleverSegEffectExtension


#
# CleverSegEffectWidget
#

class CleverSegEffectWidget:
    def __init__(self, parent=None):
        self.parent = parent

    def setup(self):
        # don't display anything for this widget - it will be hidden anyway
        pass

    def enter(self):
        pass

    def exit(self):
        pass


