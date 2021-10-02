# pylint: disable=import-error,fixme,invalid-name,too-many-locals,too-many-arguments,too-many-statements,too-many-public-methods
# ==============================================================================
#  Copyright Intel Corporation
#
#  SPDX-License-Identifier: MIT
# ==============================================================================
"""Test Extention Buffers"""

import unittest
import pyvpl
import numpy


class Test_Ext(unittest.TestCase):
    """Test Extention Buffers"""
    def check_scalar(self, obj, field, expected=None, alternate=None):
        """Check that Scalar fields behave as expected"""
        # Type speciic defaults
        if expected is None:
            expected = 0
        if alternate is None:
            alternate = 1

        # check copy
        c = obj.get()
        c2 = obj.get()
        cvalue = getattr(c, field)
        c2value = getattr(c2, field)
        # Copies should start out with the expected value
        self.assertEqual(cvalue, expected)
        self.assertEqual(c2value, expected)

        # setting value one one copy should not affect the other
        setattr(c, field, alternate)
        cvalue = getattr(c, field)
        c2value = getattr(c2, field)
        self.assertEqual(cvalue, alternate)
        self.assertEqual(c2value, expected)

        # reset
        setattr(c, field, expected)

        # check refrence
        r = obj.ref()
        r2 = obj.ref()
        rvalue = getattr(r, field)
        r2value = getattr(r2, field)

        # Copies should start out with the expected value
        self.assertEqual(rvalue, expected)
        self.assertEqual(r2value, expected)

        # setting value one one copy should affect the other, bu not copies
        setattr(r, field, alternate)
        rvalue = getattr(r, field)
        r2value = getattr(r2, field)
        self.assertEqual(rvalue, alternate)
        self.assertEqual(r2value, alternate)
        cvalue = getattr(c, field)
        c2value = getattr(c2, field)
        self.assertEqual(cvalue, expected)
        self.assertEqual(c2value, expected)

        setattr(r, field, expected)

    def check_pair(self, obj, field, expected=None, alternate=None):
        """Check that Pair fields behave as expected"""
        # Type speciic defaults
        if expected is None:
            expected = pyvpl.mfxI16Pair(0, 0)
        if alternate is None:
            alternate = pyvpl.mfxI16Pair(1, 1)

        # check copy
        c = obj.get()
        c2 = obj.get()
        cvalue = getattr(c, field)
        c2value = getattr(c2, field)
        # Copies should start out with the expected value
        self.assertEqual(cvalue, expected)
        self.assertEqual(c2value, expected)

        # setting value one one copy should not affect the other
        setattr(c, field, alternate)
        cvalue = getattr(c, field)
        c2value = getattr(c2, field)
        self.assertEqual(cvalue, alternate)
        self.assertEqual(c2value, expected)

        # reset
        setattr(c, field, expected)

        # check refrence
        r = obj.ref()
        r2 = obj.ref()
        rvalue = getattr(r, field)
        r2value = getattr(r2, field)

        # Copies should start out with the expected value
        self.assertEqual(rvalue, expected)
        self.assertEqual(r2value, expected)

        # setting value one one copy should affect the other, bu not copies
        setattr(r, field, alternate)
        rvalue = getattr(r, field)
        r2value = getattr(r2, field)
        self.assertEqual(rvalue, alternate)
        self.assertEqual(r2value, alternate)
        cvalue = getattr(c, field)
        c2value = getattr(c2, field)
        self.assertEqual(cvalue, expected)
        self.assertEqual(c2value, expected)

        setattr(r, field, expected)

    # pylint: disable=no-self-use
    def check_array(self, obj, field, expected=None, alternate=None):
        """Check that Array fields behave as expected"""
        del obj
        del field
        del expected
        del alternate

    def check_custom(self, obj, field, expected, alternate):
        """Check that custom fields behave as expected"""
        # Type speciic defaults
        if expected is None:
            expected = pyvpl.mfxI16Pair(0, 0)
        if alternate is None:
            alternate = pyvpl.mfxI16Pair(1, 1)

        # check copy
        c = obj.get()
        c2 = obj.get()
        cvalue = getattr(c, field)
        c2value = getattr(c2, field)
        # Copies should start out with the expected value
        self.assertEqual(cvalue, expected)
        self.assertEqual(c2value, expected)

        # setting value one one copy should not affect the other
        setattr(c, field, alternate)
        cvalue = getattr(c, field)
        c2value = getattr(c2, field)
        self.assertEqual(cvalue, alternate)
        self.assertEqual(c2value, expected)

        # reset
        setattr(c, field, expected)

        # check refrence
        r = obj.ref()
        r2 = obj.ref()
        rvalue = getattr(r, field)
        r2value = getattr(r2, field)

        # Copies should start out with the expected value
        self.assertEqual(rvalue, expected)
        self.assertEqual(r2value, expected)

        # setting value one one copy should affect the other, bu not copies
        setattr(r, field, alternate)
        rvalue = getattr(r, field)
        r2value = getattr(r2, field)
        self.assertEqual(rvalue, alternate)
        self.assertEqual(r2value, alternate)
        cvalue = getattr(c, field)
        c2value = getattr(c2, field)
        self.assertEqual(cvalue, expected)
        self.assertEqual(c2value, expected)

        setattr(r, field, expected)

    def check_field(self, obj, field, expected=None, alternate=None):
        """Check that fields behave as expected"""
        fld = getattr(obj.ref(), field)
        if expected is not None and alternate is not None:
            self.check_custom(obj, field, expected, alternate)
        elif isinstance(fld, (int, float)):
            self.check_scalar(obj, field, expected, alternate)
        elif isinstance(fld, pyvpl.mfxI16Pair):
            self.check_pair(obj, field, expected, alternate)
        elif isinstance(fld, numpy.ndarray):
            self.check_array(obj, field, expected, alternate)
        elif isinstance(fld, type(None)):
            pass
        else:
            print(
                f"Unknwon type {type(fld)}, skipping field check for {field}")

    def test_ExtAVCEncodedFrameInfo(self):
        """Test ExtAVCEncodedFrameInfo"""
        obj = pyvpl.ExtAVCEncodedFrameInfo()
        self.assertEqual(obj.ID, 1229344325)
        self.assertEqual(obj.size, 1056)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1229344325, 1056),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'BRCPanicMode')
        self.check_field(obj, 'FrameOrder')
        self.check_field(obj, 'LongTermIdx')
        self.check_field(obj, 'MAD')
        self.check_field(obj, 'PicStruct')
        self.check_field(obj, 'QP')
        self.check_field(obj, 'SecondFieldOffset')
        self.check_field(obj, 'UsedRefListL0')
        self.check_field(obj, 'UsedRefListL1')

    def test_ExtAVCRefListCtrl(self):
        """Test ExtAVCRefListCtrl"""
        obj = pyvpl.ExtAVCRefListCtrl()
        self.assertEqual(obj.ID, 1414745170)
        self.assertEqual(obj.size, 1068)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1414745170, 1068),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'ApplyLongTermIdx')
        self.check_field(obj, 'LongTermRefList')
        self.check_field(obj, 'NumRefIdxL0Active')
        self.check_field(obj, 'NumRefIdxL1Active')
        self.check_field(obj, 'PreferredRefList')
        self.check_field(obj, 'RejectedRefList')

    def test_ExtAVCRefLists(self):
        """Test ExtAVCRefLists"""
        obj = pyvpl.ExtAVCRefLists()
        self.assertEqual(obj.ID, 1398033490)
        self.assertEqual(obj.size, 1040)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1398033490, 1040),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'NumRefIdxL0Active')
        self.check_field(obj, 'NumRefIdxL1Active')
        self.check_field(obj, 'RefPicList0')
        self.check_field(obj, 'RefPicList1')

    def test_ExtAVCRoundingOffset(self):
        """Test ExtAVCRoundingOffset"""
        obj = pyvpl.ExtAVCRoundingOffset()
        self.assertEqual(obj.ID, 1329876562)
        self.assertEqual(obj.size, 64)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1329876562, 64),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'EnableRoundingInter')
        self.check_field(obj, 'EnableRoundingIntra')
        self.check_field(obj, 'RoundingOffsetInter')
        self.check_field(obj, 'RoundingOffsetIntra')

    def test_ExtAvcTemporalLayers(self):
        """Test ExtAvcTemporalLayers"""
        obj = pyvpl.ExtAvcTemporalLayers()
        self.assertEqual(obj.ID, 1280136257)
        self.assertEqual(obj.size, 92)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1280136257, 92),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'BaseLayerPID')
        self.check_field(obj, 'Layer')

    def test_ExtChromaLocInfo(self):
        """Test ExtChromaLocInfo"""
        obj = pyvpl.ExtChromaLocInfo()
        self.assertEqual(obj.ID, 1313426499)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1313426499, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'ChromaLocInfoPresentFlag')
        self.check_field(obj, 'ChromaSampleLocTypeBottomField')
        self.check_field(obj, 'ChromaSampleLocTypeTopField')

    def test_ExtCodingOption(self):
        """Test ExtCodingOption"""
        obj = pyvpl.ExtCodingOption()
        self.assertEqual(obj.ID, 1347372099)
        self.assertEqual(obj.size, 64)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1347372099, 64),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'AUDelimiter')
        self.check_field(obj, 'CAVLC')
        self.check_field(obj, 'EndOfSequence')
        self.check_field(obj, 'EndOfStream')
        self.check_field(obj, 'FieldOutput')
        self.check_field(obj, 'FramePicture')
        self.check_field(obj, 'InterPredBlockSize')
        self.check_field(obj, 'IntraPredBlockSize')
        self.check_field(obj, 'MECostType')
        self.check_field(obj, 'MESearchType')
        self.check_field(obj, 'MVPrecision')
        self.check_field(obj, 'MVSearchWindow')
        self.check_field(obj, 'MaxDecFrameBuffering')
        self.check_field(obj, 'NalHrdConformance')
        self.check_field(obj, 'PicTimingSEI')
        self.check_field(obj, 'RateDistortionOpt')
        self.check_field(obj, 'RecoveryPointSEI')
        self.check_field(obj, 'RefPicListReordering')
        self.check_field(obj, 'RefPicMarkRep')
        self.check_field(obj, 'ResetRefList')
        self.check_field(obj, 'SingleSeiNalUnit')
        self.check_field(obj, 'ViewOutput')
        self.check_field(obj, 'VuiNalHrdParameters')
        self.check_field(obj, 'VuiVclHrdParameters')

    def test_ExtCodingOption2(self):
        """Test ExtCodingOption2"""
        obj = pyvpl.ExtCodingOption2()
        self.assertEqual(obj.ID, 844055619)
        self.assertEqual(obj.size, 68)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(844055619, 68),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'AdaptiveB')
        self.check_field(obj, 'AdaptiveI')
        self.check_field(obj, 'BRefType')
        self.check_field(obj, 'BitrateLimit')
        self.check_field(obj, 'BufferingPeriodSEI')
        self.check_field(obj, 'DisableDeblockingIdc')
        self.check_field(obj, 'DisableVUI')
        self.check_field(obj, 'EnableMAD')
        self.check_field(obj, 'ExtBRC')
        self.check_field(obj, 'FixedFrameRate')
        self.check_field(obj, 'IntRefCycleSize')
        self.check_field(obj, 'IntRefQPDelta')
        self.check_field(obj, 'IntRefType')
        self.check_field(obj, 'LookAheadDS')
        self.check_field(obj, 'LookAheadDepth')
        self.check_field(obj, 'MBBRC')
        self.check_field(obj, 'MaxFrameSize')
        self.check_field(obj, 'MaxQPB')
        self.check_field(obj, 'MaxQPI')
        self.check_field(obj, 'MaxQPP')
        self.check_field(obj, 'MaxSliceSize')
        self.check_field(obj, 'MinQPB')
        self.check_field(obj, 'MinQPI')
        self.check_field(obj, 'MinQPP')
        self.check_field(obj, 'NumMbPerSlice')
        self.check_field(obj, 'RepeatPPS')
        self.check_field(obj, 'SkipFrame')
        self.check_field(obj, 'Trellis')
        self.check_field(obj, 'UseRawRef')

    def test_ExtCodingOption3(self):
        """Test ExtCodingOption3"""
        obj = pyvpl.ExtCodingOption3()
        self.assertEqual(obj.ID, 860832835)
        self.assertEqual(obj.size, 512)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(860832835, 512),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'AdaptiveCQM')
        self.check_field(obj, 'AdaptiveMaxFrameSize')
        self.check_field(obj, 'AspectRatioInfoPresent')
        self.check_field(obj, 'BRCPanicMode')
        self.check_field(obj, 'BitstreamRestriction')
        self.check_field(obj, 'ContentInfo')
        self.check_field(obj, 'DirectBiasAdjustment')
        self.check_field(obj, 'EnableMBForceIntra')
        self.check_field(obj, 'EnableMBQP')
        self.check_field(obj, 'EnableNalUnitType')
        self.check_field(obj, 'EnableQPOffset')
        self.check_field(obj, 'EncodedUnitsInfo')
        self.check_field(obj, 'ExtBrcAdaptiveLTR')
        self.check_field(obj, 'FadeDetection')
        self.check_field(obj, 'GPB')
        self.check_field(obj, 'GlobalMotionBiasAdjustment')
        self.check_field(obj, 'IntRefCycleDist')
        self.check_field(obj, 'LowDelayBRC')
        self.check_field(obj, 'LowDelayHrd')
        self.check_field(obj, 'MBDisableSkipMap')
        self.check_field(obj, 'MVCostScalingFactor')
        self.check_field(obj, 'MaxFrameSizeI')
        self.check_field(obj, 'MaxFrameSizeP')
        self.check_field(obj, 'MotionVectorsOverPicBoundaries')
        self.check_field(obj, 'NumRefActiveBL0')
        self.check_field(obj, 'NumRefActiveBL1')
        self.check_field(obj, 'NumRefActiveP')
        self.check_field(obj, 'NumSliceB')
        self.check_field(obj, 'NumSliceI')
        self.check_field(obj, 'NumSliceP')
        self.check_field(obj, 'OverscanAppropriate')
        self.check_field(obj, 'OverscanInfoPresent')
        self.check_field(obj, 'PRefType')
        self.check_field(obj, 'QPOffset')
        self.check_field(obj, 'QVBRQuality')
        self.check_field(obj, 'RepartitionCheckEnable')
        self.check_field(obj, 'ScenarioInfo')
        self.check_field(obj, 'TargetBitDepthChroma')
        self.check_field(obj, 'TargetBitDepthLuma')
        self.check_field(obj, 'TargetChromaFormatPlus1')
        self.check_field(obj, 'TimingInfoPresent')
        self.check_field(obj, 'TransformSkip')
        self.check_field(obj, 'WeightedBiPred')
        self.check_field(obj, 'WeightedPred')
        self.check_field(obj, 'WinBRCMaxAvgKbps')
        self.check_field(obj, 'WinBRCSize')

    def test_ExtCodingOptionSPSPPS(self):
        """Test ExtCodingOptionSPSPPS"""
        obj = pyvpl.ExtCodingOptionSPSPPS()
        self.assertEqual(obj.ID, 1347637059)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1347637059, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'PPSBufSize')
        self.check_field(obj, 'PPSBuffer')
        self.check_field(obj, 'PPSId')
        self.check_field(obj, 'SPSBufSize')
        self.check_field(obj, 'SPSBuffer')
        self.check_field(obj, 'SPSId')

    def test_ExtCodingOptionVPS(self):
        """Test ExtCodingOptionVPS"""
        obj = pyvpl.ExtCodingOptionVPS()
        self.assertEqual(obj.ID, 1347833667)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1347833667, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'VPSBufSize')
        self.check_field(obj, 'VPSBuffer')
        self.check_field(obj, 'VPSId')

    def test_ExtColorConversion(self):
        """Test ExtColorConversion"""
        obj = pyvpl.ExtColorConversion()
        self.assertEqual(obj.ID, 1129530198)
        self.assertEqual(obj.size, 64)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1129530198, 64),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'ChromaSiting')

    def test_ExtContentLightLevelInfo(self):
        """Test ExtContentLightLevelInfo"""
        obj = pyvpl.ExtContentLightLevelInfo()
        self.assertEqual(obj.ID, 1397312588)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1397312588, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'InsertPayloadToggle')
        self.check_field(obj, 'MaxContentLightLevel')
        self.check_field(obj, 'MaxPicAverageLightLevel')

    def test_ExtDecVideoProcessing(self):
        """Test ExtDecVideoProcessing"""
        obj = pyvpl.ExtDecVideoProcessing()
        self.assertEqual(obj.ID, 1447249220)
        self.assertEqual(obj.size, 132)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1447249220, 132),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'In')
        self.check_field(obj, 'Out')

    def test_ExtDecodeErrorReport(self):
        """Test ExtDecodeErrorReport"""
        obj = pyvpl.ExtDecodeErrorReport()
        self.assertEqual(obj.ID, 1381123396)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1381123396, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'ErrorTypes')

    def test_ExtDecodedFrameInfo(self):
        """Test ExtDecodedFrameInfo"""
        obj = pyvpl.ExtDecodedFrameInfo()
        self.assertEqual(obj.ID, 1229342020)
        self.assertEqual(obj.size, 128)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1229342020, 128),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'FrameType')

    def test_ExtDeviceAffinityMask(self):
        """Test ExtDeviceAffinityMask"""
        obj = pyvpl.ExtDeviceAffinityMask()
        self.assertEqual(obj.ID, 1296449860)
        self.assertEqual(obj.size, 168)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1296449860, 168),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'DeviceID')
        self.check_field(obj, 'NumSubDevices')

    def test_ExtDirtyRect(self):
        """Test ExtDirtyRect"""
        obj = pyvpl.ExtDirtyRect()
        self.assertEqual(obj.ID, 1229935172)
        self.assertEqual(obj.size, 8224)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1229935172, 8224),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'NumRect')
        self.check_field(obj, 'Rect')

    def test_ExtEncodedSlicesInfo(self):
        """Test ExtEncodedSlicesInfo"""
        obj = pyvpl.ExtEncodedSlicesInfo()
        self.assertEqual(obj.ID, 1230196293)
        self.assertEqual(obj.size, 64)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1230196293, 64),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'NumEncodedSlice')
        self.check_field(obj, 'NumSliceNonCopliant')
        self.check_field(obj, 'NumSliceSizeAlloc')
        self.check_field(obj, 'SliceSize')
        self.check_field(obj, 'SliceSizeOverflow')

    def test_ExtEncodedUnitsInfo(self):
        """Test ExtEncodedUnitsInfo"""
        obj = pyvpl.ExtEncodedUnitsInfo()
        self.assertEqual(obj.ID, 1230327365)
        self.assertEqual(obj.size, 64)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1230327365, 64),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'NumUnitsAlloc')
        self.check_field(obj, 'NumUnitsEncoded')
        self.check_field(obj, 'UnitInfo')

    def test_ExtEncoderCapability(self):
        """Test ExtEncoderCapability"""
        obj = pyvpl.ExtEncoderCapability()
        self.assertEqual(obj.ID, 1346588229)
        self.assertEqual(obj.size, 128)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1346588229, 128),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'MBPerSec')

    def test_ExtEncoderIPCMArea(self):
        """Test ExtEncoderIPCMArea"""
        obj = pyvpl.ExtEncoderIPCMArea()
        self.assertEqual(obj.ID, 1380795216)
        self.assertEqual(obj.size, 40)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1380795216, 40),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'Areas')
        self.check_field(obj, 'NumArea')

    def test_ExtEncoderROI(self):
        """Test ExtEncoderROI"""
        obj = pyvpl.ExtEncoderROI()
        self.assertEqual(obj.ID, 1229935173)
        self.assertEqual(obj.size, 8224)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1229935173, 8224),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'NumROI')
        self.check_field(obj, 'ROI')
        self.check_field(obj, 'ROIMode')

    def test_ExtEncoderResetOption(self):
        """Test ExtEncoderResetOption"""
        obj = pyvpl.ExtEncoderResetOption()
        self.assertEqual(obj.ID, 1330794053)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1330794053, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'StartNewSequence')

    def test_ExtHEVCParam(self):
        """Test ExtHEVCParam"""
        obj = pyvpl.ExtHEVCParam()
        self.assertEqual(obj.ID, 1345664562)
        self.assertEqual(obj.size, 256)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1345664562, 256),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'GeneralConstraintFlags')
        self.check_field(obj, 'LCUSize')
        self.check_field(obj, 'PicHeightInLumaSamples')
        self.check_field(obj, 'PicWidthInLumaSamples')
        self.check_field(obj, 'SampleAdaptiveOffset')

    def test_ExtHEVCRegion(self):
        """Test ExtHEVCRegion"""
        obj = pyvpl.ExtHEVCRegion()
        self.assertEqual(obj.ID, 1379218994)
        self.assertEqual(obj.size, 64)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1379218994, 64),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'RegionEncoding')
        self.check_field(obj, 'RegionId')
        self.check_field(obj, 'RegionType')

    def test_ExtHEVCTiles(self):
        """Test ExtHEVCTiles"""
        obj = pyvpl.ExtHEVCTiles()
        self.assertEqual(obj.ID, 1412773426)
        self.assertEqual(obj.size, 160)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1412773426, 160),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'NumTileColumns')
        self.check_field(obj, 'NumTileRows')

    def test_ExtInsertHeaders(self):
        """Test ExtInsertHeaders"""
        obj = pyvpl.ExtInsertHeaders()
        self.assertEqual(obj.ID, 1163022419)
        self.assertEqual(obj.size, 28)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1163022419, 28),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'PPS')
        self.check_field(obj, 'SPS')

    def test_ExtMBDisableSkipMap(self):
        """Test ExtMBDisableSkipMap"""
        obj = pyvpl.ExtMBDisableSkipMap()
        self.assertEqual(obj.ID, 1297302605)
        self.assertEqual(obj.size, 64)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1297302605, 64),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'Map')
        self.check_field(obj, 'MapSize')

    def test_ExtMBForceIntra(self):
        """Test ExtMBForceIntra"""
        obj = pyvpl.ExtMBForceIntra()
        self.assertEqual(obj.ID, 1229341261)
        self.assertEqual(obj.size, 64)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1229341261, 64),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'Map')
        self.check_field(obj, 'MapSize')

    def test_ExtMBQP(self):
        """Test ExtMBQP"""
        obj = pyvpl.ExtMBQP()
        self.assertEqual(obj.ID, 1347502669)
        self.assertEqual(obj.size, 64)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1347502669, 64),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'BlockSize')
        self.check_field(obj, 'DeltaQP')
        self.check_field(obj, 'Mode')
        self.check_field(obj, 'NumQPAlloc')
        self.check_field(obj, 'QP')
        self.check_field(obj, 'QPmode')

    def test_ExtMVOverPicBoundaries(self):
        """Test ExtMVOverPicBoundaries"""
        obj = pyvpl.ExtMVOverPicBoundaries()
        self.assertEqual(obj.ID, 1112561229)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1112561229, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'StickBottom')
        self.check_field(obj, 'StickLeft')
        self.check_field(obj, 'StickRight')
        self.check_field(obj, 'StickTop')

    def test_ExtMasteringDisplayColourVolume(self):
        """Test ExtMasteringDisplayColourVolume"""
        obj = pyvpl.ExtMasteringDisplayColourVolume()
        self.assertEqual(obj.ID, 1398162244)
        self.assertEqual(obj.size, 64)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1398162244, 64),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'DisplayPrimariesX')
        self.check_field(obj, 'DisplayPrimariesY')
        self.check_field(obj, 'InsertPayloadToggle')
        self.check_field(obj, 'MaxDisplayMasteringLuminance')
        self.check_field(obj, 'MinDisplayMasteringLuminance')
        self.check_field(obj, 'WhitePointX')
        self.check_field(obj, 'WhitePointY')

    def test_ExtMoveRect(self):
        """Test ExtMoveRect"""
        obj = pyvpl.ExtMoveRect()
        self.assertEqual(obj.ID, 1229935181)
        self.assertEqual(obj.size, 8224)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1229935181, 8224),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'NumRect')
        self.check_field(obj, 'Rect')

    def test_ExtPartialBitstreamParam(self):
        """Test ExtPartialBitstreamParam"""
        obj = pyvpl.ExtPartialBitstreamParam()
        self.assertEqual(obj.ID, 1347371600)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1347371600, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'BlockSize')
        self.check_field(obj, 'Granularity')

    def test_ExtPictureTimingSEI(self):
        """Test ExtPictureTimingSEI"""
        obj = pyvpl.ExtPictureTimingSEI()
        self.assertEqual(obj.ID, 1163088976)
        self.assertEqual(obj.size, 160)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1163088976, 160),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'TimeStamp')

    def test_ExtPredWeightTable(self):
        """Test ExtPredWeightTable"""
        obj = pyvpl.ExtPredWeightTable()
        self.assertEqual(obj.ID, 1415008325)
        self.assertEqual(obj.size, 1152)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1415008325, 1152),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'ChromaLog2WeightDenom')
        self.check_field(obj, 'LumaLog2WeightDenom')
        self.check_field(obj, 'ChromaWeightFlag')
        self.check_field(obj, 'LumaWeightFlag')
        self.check_field(obj, 'Weight')
        self.check_field(obj, 'ChromaWeightFlag')
        self.check_field(obj, 'LumaWeightFlag')
        self.check_field(obj, 'Weight')

    def test_ExtTimeCode(self):
        """Test ExtTimeCode"""
        obj = pyvpl.ExtTimeCode()
        self.assertEqual(obj.ID, 1145261396)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1145261396, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'DropFrameFlag')
        self.check_field(obj, 'TimeCodeHours')
        self.check_field(obj, 'TimeCodeMinutes')
        self.check_field(obj, 'TimeCodePictures')
        self.check_field(obj, 'TimeCodeSeconds')

    def test_ExtVP9Param(self):
        """Test ExtVP9Param"""
        obj = pyvpl.ExtVP9Param()
        self.assertEqual(obj.ID, 1380012089)
        self.assertEqual(obj.size, 256)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1380012089, 256),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'FrameHeight')
        self.check_field(obj, 'FrameWidth')
        self.check_field(obj, 'NumTileColumns')
        self.check_field(obj, 'NumTileRows')
        self.check_field(obj, 'QIndexDeltaChromaAC')
        self.check_field(obj, 'QIndexDeltaChromaDC')
        self.check_field(obj, 'QIndexDeltaLumaDC')
        self.check_field(obj, 'WriteIVFHeaders')

    def test_ExtVP9Segmentation(self):
        """Test ExtVP9Segmentation"""
        obj = pyvpl.ExtVP9Segmentation()
        self.assertEqual(obj.ID, 1195725625)
        self.assertEqual(obj.size, 384)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1195725625, 384),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'NumSegmentIdAlloc')
        self.check_field(obj, 'NumSegments')
        self.check_field(obj, 'Segment')
        self.check_field(obj, 'SegmentId')
        self.check_field(obj, 'SegmentIdBlockSize')

    def test_ExtVP9TemporalLayers(self):
        """Test ExtVP9TemporalLayers"""
        obj = pyvpl.ExtVP9TemporalLayers()
        self.assertEqual(obj.ID, 1280136249)
        self.assertEqual(obj.size, 384)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1280136249, 384),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'Layer')

    def test_ExtVPPColorFill(self):
        """Test ExtVPPColorFill"""
        obj = pyvpl.ExtVPPColorFill()
        self.assertEqual(obj.ID, 1179403094)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1179403094, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'Enable')

    def test_ExtVPPComposite(self):
        """Test ExtVPPComposite"""
        obj = pyvpl.ExtVPPComposite()
        self.assertEqual(obj.ID, 1347833667)
        self.assertEqual(obj.size, 72)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1347833667, 72),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'B')
        self.check_field(obj, 'G')
        self.check_field(obj, 'NumInputStream')
        self.check_field(obj, 'NumTiles')
        self.check_field(obj, 'R')
        self.check_field(obj, 'U')
        self.check_field(obj, 'V')
        self.check_field(obj, 'Y')

    def test_ExtVPPDeinterlacing(self):
        """Test ExtVPPDeinterlacing"""
        obj = pyvpl.ExtVPPDeinterlacing()
        self.assertEqual(obj.ID, 1229213782)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1229213782, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'Mode')
        self.check_field(obj, 'TelecineLocation')
        self.check_field(obj, 'TelecinePattern')

    def test_ExtVPPDenoise(self):
        """Test ExtVPPDenoise"""
        obj = pyvpl.ExtVPPDenoise()
        self.assertEqual(obj.ID, 1397313092)
        self.assertEqual(obj.size, 12)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1397313092, 12),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'DenoiseFactor')

    def test_ExtVPPDetail(self):
        """Test ExtVPPDetail"""
        obj = pyvpl.ExtVPPDetail()
        self.assertEqual(obj.ID, 542393668)
        self.assertEqual(obj.size, 12)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(542393668, 12),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'DetailFactor')

    def test_ExtVPPDoNotUse(self):
        """Test ExtVPPDoNotUse"""
        obj = pyvpl.ExtVPPDoNotUse()
        self.assertEqual(obj.ID, 1163089230)
        self.assertEqual(obj.size, 24)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1163089230, 24),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'AlgList')
        self.check_field(obj, 'NumAlg')

    def test_ExtVPPDoUse(self):
        """Test ExtVPPDoUse"""
        obj = pyvpl.ExtVPPDoUse()
        self.assertEqual(obj.ID, 1163089220)
        self.assertEqual(obj.size, 24)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1163089220, 24),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'AlgList')
        self.check_field(obj, 'NumAlg')

    def test_ExtVPPFieldProcessing(self):
        """Test ExtVPPFieldProcessing"""
        obj = pyvpl.ExtVPPFieldProcessing()
        self.assertEqual(obj.ID, 1330794566)
        self.assertEqual(obj.size, 64)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1330794566, 64),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'InField')
        self.check_field(obj, 'Mode')
        self.check_field(obj, 'OutField')

    def test_ExtVPPFrameRateConversion(self):
        """Test ExtVPPFrameRateConversion"""
        obj = pyvpl.ExtVPPFrameRateConversion()
        self.assertEqual(obj.ID, 541282886)
        self.assertEqual(obj.size, 72)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(541282886, 72),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'Algorithm')

    def test_ExtVPPImageStab(self):
        """Test ExtVPPImageStab"""
        obj = pyvpl.ExtVPPImageStab()
        self.assertEqual(obj.ID, 1112822601)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1112822601, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'Mode')

    def test_ExtVPPMirroring(self):
        """Test ExtVPPMirroring"""
        obj = pyvpl.ExtVPPMirroring()
        self.assertEqual(obj.ID, 1381124429)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1381124429, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'Type')

    def test_ExtVPPProcAmp(self):
        """Test ExtVPPProcAmp"""
        obj = pyvpl.ExtVPPProcAmp()
        self.assertEqual(obj.ID, 1347240272)
        self.assertEqual(obj.size, 40)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1347240272, 40),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'Brightness')
        self.check_field(obj, 'Contrast')
        self.check_field(obj, 'Hue')
        self.check_field(obj, 'Saturation')

    def test_ExtVPPRotation(self):
        """Test ExtVPPRotation"""
        obj = pyvpl.ExtVPPRotation()
        self.assertEqual(obj.ID, 542396242)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(542396242, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'Angle')

    def test_ExtVPPScaling(self):
        """Test ExtVPPScaling"""
        obj = pyvpl.ExtVPPScaling()
        self.assertEqual(obj.ID, 1279480662)
        self.assertEqual(obj.size, 32)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1279480662, 32),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'InterpolationMethod')
        self.check_field(obj, 'ScalingMode')

    def test_ExtVPPVideoSignalInfo(self):
        """Test ExtVPPVideoSignalInfo"""
        obj = pyvpl.ExtVPPVideoSignalInfo()
        self.assertEqual(obj.ID, 1230198358)
        self.assertEqual(obj.size, 48)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1230198358, 48),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'In')
        self.check_field(obj, 'NominalRange')
        self.check_field(obj, 'Out')
        self.check_field(obj, 'TransferMatrix')

    def test_ExtVideoSignalInfo(self):
        """Test ExtVideoSignalInfo"""
        obj = pyvpl.ExtVideoSignalInfo()
        self.assertEqual(obj.ID, 1313428310)
        self.assertEqual(obj.size, 20)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1313428310, 20),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'ColourDescriptionPresent')
        self.check_field(obj, 'ColourPrimaries')
        self.check_field(obj, 'MatrixCoefficients')
        self.check_field(obj, 'TransferCharacteristics')
        self.check_field(obj, 'VideoFormat')
        self.check_field(obj, 'VideoFullRange')

    def test_ExtVppAuxData(self):
        """Test ExtVppAuxData"""
        # TODO: Debug why values are being set
        obj = pyvpl.ExtVppAuxData()
        self.assertEqual(obj.ID, 1146639681)
        self.assertEqual(obj.size, 20)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1146639681, 20),
                         pyvpl.mfxExtBuffer(1, 1))
        # self.check_field(obj, 'PicStruct')
        self.check_field(obj, 'RepeatedFrame')
        self.check_field(obj, 'SceneChangeRate')
        # self.check_field(obj, 'SpatialComplexity')
        self.check_field(obj, 'TemporalComplexity')

    def test_ExtVppMctf(self):
        """Test ExtVppMctf"""
        obj = pyvpl.ExtVppMctf()
        self.assertEqual(obj.ID, 1179927373)
        self.assertEqual(obj.size, 64)
        self.check_field(obj, 'Header', pyvpl.mfxExtBuffer(1179927373, 64),
                         pyvpl.mfxExtBuffer(1, 1))
        self.check_field(obj, 'FilterStrength')


if __name__ == '__main__':
    unittest.main()
