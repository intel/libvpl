<!-- [[[cog
"""
This is logic to automatically generate the list of keys below.

regenerate the following code with:
  cog -cPr api/strings.md

to install cog:
  pip install cogapp
"""

import cog
import csv

def gen_key_name(field_name):
    if field_name.startswith('mfxVideoParam.mfx.FrameInfo.'):
        return field_name[len('mfxVideoParam.mfx.FrameInfo.'):]
    if field_name.startswith('mfxVideoParam.mfx.'):
        return field_name[len('mfxVideoParam.mfx.'):]
    if field_name.startswith('mfxVideoParam.vpp.In.FrameInfo.'):
        return 'VPPIn' + field_name[len('mfxVideoParam.vpp.In.FrameInfo.'):]
    if field_name.startswith('mfxVideoParam.vpp.Out.FrameInfo.'):
        return 'VPPOut' + field_name[len('mfxVideoParam.vpp.Out.FrameInfo.'):]
    if field_name.startswith('mfxVideoParam.'):
        return field_name[len('mfxVideoParam.'):]
    if field_name.startswith('mfxExt'):
        return field_name
    return '!!!!!!!!!!!!!!!!'

def code():
    with open('api/strings.csv') as csv_file:
        reader = csv.DictReader(csv_file)
        rows = []
        for row in reader:
            field_type = row['type']
            field_name = row['name']
            if row['category']:
                field_category = row['category'].split(':')
            else:
                field_category = []

            # translate the field name to the String API name
            key_name = gen_key_name(field_name)

            # identify the name that will hold the value
            field_names = field_name.split('.')
            prop_name = '.'.join(field_names[1:])
            if field_name.startswith('mfxExt'):
                ext_type = field_names[0]
            else:
                ext_type = 'mfxVideoParam'

            # clear the name of any keys we don't have support for yet
            if field_category:
                key_name = ''

            fields = [
                    key_name,
                    f'`{field_type}`',
                    f'`{ext_type}.{prop_name}`',
                    ",".join(field_category)
                    ]
            rows.append(fields)

        headers = ['Key', 'Type', 'Field', 'Category']
        row_widths = [0, 0, 0, 0]

        # get column widths
        for i in range(len(headers)): 
            row_widths[i] = max(row_widths[i], len(headers[i]))
        for row in rows: 
            for i in range(len(row)): 
                row_widths[i] = max(row_widths[i], len(row[i]))

        # print headers
        fields = []
        for i in range(len(headers)): 
            fields.append(headers[i].ljust(row_widths[i], ' '))
        print(f'| {" | ".join(fields)} |')

        fields = []
        for i in range(len(headers)): 
            fields.append('-'*(row_widths[i]+2))
        print(f'|{"|".join(fields)}|')

        # print table
        for row in rows: 
            fields = []
            for i in range(len(row)): 
                fields.append(row[i].ljust(row_widths[i], ' '))
            print(f'| {" | ".join(fields)} |')

code()
]]] -->
| Key                                                             | Type        | Field                                                             | Category      |
|-----------------------------------------------------------------|-------------|-------------------------------------------------------------------|---------------|
| AllocId                                                         | `mfxU32`    | `mfxVideoParam.AllocId`                                           |               |
| AsyncDepth                                                      | `mfxU16`    | `mfxVideoParam.AsyncDepth`                                        |               |
| LowPower                                                        | `mfxU16`    | `mfxVideoParam.mfx.LowPower`                                      |               |
| BRCParamMultiplier                                              | `mfxU16`    | `mfxVideoParam.mfx.BRCParamMultiplier`                            |               |
| ChannelId                                                       | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.ChannelId`                           |               |
| BitDepthLuma                                                    | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.BitDepthLuma`                        |               |
| BitDepthChroma                                                  | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.BitDepthChroma`                      |               |
| Shift                                                           | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.Shift`                               |               |
| FrameId.TemporalId                                              | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.FrameId.TemporalId`                  |               |
| FrameId.PriorityId                                              | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.FrameId.PriorityId`                  |               |
| FrameId.DependencyId                                            | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.FrameId.DependencyId`                |               |
| FrameId.QualityId                                               | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.FrameId.QualityId`                   |               |
| FrameId.ViewId                                                  | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.FrameId.ViewId`                      |               |
| FourCC                                                          | `mfxU32`    | `mfxVideoParam.mfx.FrameInfo.FourCC`                              |               |
| Width                                                           | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.Width`                               |               |
| Height                                                          | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.Height`                              |               |
| CropX                                                           | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.CropX`                               |               |
| CropY                                                           | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.CropY`                               |               |
| CropW                                                           | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.CropW`                               |               |
| CropH                                                           | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.CropH`                               |               |
| BufferSize                                                      | `mfxU64`    | `mfxVideoParam.mfx.FrameInfo.BufferSize`                          |               |
| FrameRateExtN                                                   | `mfxU32`    | `mfxVideoParam.mfx.FrameInfo.FrameRateExtN`                       |               |
| FrameRateExtD                                                   | `mfxU32`    | `mfxVideoParam.mfx.FrameInfo.FrameRateExtD`                       |               |
| AspectRatioW                                                    | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.AspectRatioW`                        |               |
| AspectRatioH                                                    | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.AspectRatioH`                        |               |
| PicStruct                                                       | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.PicStruct`                           |               |
| ChromaFormat                                                    | `mfxU16`    | `mfxVideoParam.mfx.FrameInfo.ChromaFormat`                        |               |
| CodecId                                                         | `mfxU32`    | `mfxVideoParam.mfx.CodecId`                                       |               |
| CodecProfile                                                    | `mfxU16`    | `mfxVideoParam.mfx.CodecProfile`                                  |               |
| CodecLevel                                                      | `mfxU16`    | `mfxVideoParam.mfx.CodecLevel`                                    |               |
| NumThread                                                       | `mfxU16`    | `mfxVideoParam.mfx.NumThread`                                     |               |
| TargetUsage                                                     | `mfxU16`    | `mfxVideoParam.mfx.TargetUsage`                                   |               |
| GopPicSize                                                      | `mfxU16`    | `mfxVideoParam.mfx.GopPicSize`                                    |               |
| GopRefDist                                                      | `mfxU16`    | `mfxVideoParam.mfx.GopRefDist`                                    |               |
| GopOptFlag                                                      | `mfxU16`    | `mfxVideoParam.mfx.GopOptFlag`                                    |               |
| IdrInterval                                                     | `mfxU16`    | `mfxVideoParam.mfx.IdrInterval`                                   |               |
| RateControlMethod                                               | `mfxU16`    | `mfxVideoParam.mfx.RateControlMethod`                             |               |
| InitialDelayInKB                                                | `mfxU16`    | `mfxVideoParam.mfx.InitialDelayInKB`                              |               |
| QPI                                                             | `mfxU16`    | `mfxVideoParam.mfx.QPI`                                           |               |
| Accuracy                                                        | `mfxU16`    | `mfxVideoParam.mfx.Accuracy`                                      |               |
| BufferSizeInKB                                                  | `mfxU16`    | `mfxVideoParam.mfx.BufferSizeInKB`                                |               |
| TargetKbps                                                      | `mfxU16`    | `mfxVideoParam.mfx.TargetKbps`                                    |               |
| QPP                                                             | `mfxU16`    | `mfxVideoParam.mfx.QPP`                                           |               |
| ICQQuality                                                      | `mfxU16`    | `mfxVideoParam.mfx.ICQQuality`                                    |               |
| MaxKbps                                                         | `mfxU16`    | `mfxVideoParam.mfx.MaxKbps`                                       |               |
| QPB                                                             | `mfxU16`    | `mfxVideoParam.mfx.QPB`                                           |               |
| Convergence                                                     | `mfxU16`    | `mfxVideoParam.mfx.Convergence`                                   |               |
| NumSlice                                                        | `mfxU16`    | `mfxVideoParam.mfx.NumSlice`                                      |               |
| NumRefFrame                                                     | `mfxU16`    | `mfxVideoParam.mfx.NumRefFrame`                                   |               |
| EncodedOrder                                                    | `mfxU16`    | `mfxVideoParam.mfx.EncodedOrder`                                  |               |
| DecodedOrder                                                    | `mfxU16`    | `mfxVideoParam.mfx.DecodedOrder`                                  |               |
| ExtendedPicStruct                                               | `mfxU16`    | `mfxVideoParam.mfx.ExtendedPicStruct`                             |               |
| TimeStampCalc                                                   | `mfxU16`    | `mfxVideoParam.mfx.TimeStampCalc`                                 |               |
| SliceGroupsPresent                                              | `mfxU16`    | `mfxVideoParam.mfx.SliceGroupsPresent`                            |               |
| MaxDecFrameBuffering                                            | `mfxU16`    | `mfxVideoParam.mfx.MaxDecFrameBuffering`                          |               |
| EnableReallocRequest                                            | `mfxU16`    | `mfxVideoParam.mfx.EnableReallocRequest`                          |               |
| FilmGrain                                                       | `mfxU16`    | `mfxVideoParam.mfx.FilmGrain`                                     |               |
| IgnoreLevelConstrain                                            | `mfxU16`    | `mfxVideoParam.mfx.IgnoreLevelConstrain`                          |               |
| SkipOutput                                                      | `mfxU16`    | `mfxVideoParam.mfx.SkipOutput`                                    |               |
| JPEGChromaFormat                                                | `mfxU16`    | `mfxVideoParam.mfx.JPEGChromaFormat`                              |               |
| Rotation                                                        | `mfxU16`    | `mfxVideoParam.mfx.Rotation`                                      |               |
| JPEGColorFormat                                                 | `mfxU16`    | `mfxVideoParam.mfx.JPEGColorFormat`                               |               |
| InterleavedDec                                                  | `mfxU16`    | `mfxVideoParam.mfx.InterleavedDec`                                |               |
|                                                                 | `mfxU8`     | `mfxVideoParam.mfx.SamplingFactorH[4]`                            | array         |
|                                                                 | `mfxU8`     | `mfxVideoParam.mfx.SamplingFactorV[4]`                            | array         |
| Interleaved                                                     | `mfxU16`    | `mfxVideoParam.mfx.Interleaved`                                   |               |
| Quality                                                         | `mfxU16`    | `mfxVideoParam.mfx.Quality`                                       |               |
| RestartInterval                                                 | `mfxU16`    | `mfxVideoParam.mfx.RestartInterval`                               |               |
| vpp.In.ChannelId                                                | `mfxU16`    | `mfxVideoParam.vpp.In.ChannelId`                                  |               |
| vpp.In.BitDepthLuma                                             | `mfxU16`    | `mfxVideoParam.vpp.In.BitDepthLuma`                               |               |
| vpp.In.BitDepthChroma                                           | `mfxU16`    | `mfxVideoParam.vpp.In.BitDepthChroma`                             |               |
| vpp.In.Shift                                                    | `mfxU16`    | `mfxVideoParam.vpp.In.Shift`                                      |               |
| vpp.In.FrameId.TemporalId                                       | `mfxU16`    | `mfxVideoParam.vpp.In.FrameId.TemporalId`                         |               |
| vpp.In.FrameId.PriorityId                                       | `mfxU16`    | `mfxVideoParam.vpp.In.FrameId.PriorityId`                         |               |
| vpp.In.FrameId.DependencyId                                     | `mfxU16`    | `mfxVideoParam.vpp.In.FrameId.DependencyId`                       |               |
| vpp.In.FrameId.QualityId                                        | `mfxU16`    | `mfxVideoParam.vpp.In.FrameId.QualityId`                          |               |
| vpp.In.FrameId.ViewId                                           | `mfxU16`    | `mfxVideoParam.vpp.In.FrameId.ViewId`                             |               |
| vpp.In.FourCC                                                   | `mfxU32`    | `mfxVideoParam.vpp.In.FourCC`                                     |               |
| vpp.In.Width                                                    | `mfxU16`    | `mfxVideoParam.vpp.In.Width`                                      |               |
| vpp.In.Height                                                   | `mfxU16`    | `mfxVideoParam.vpp.In.Height`                                     |               |
| vpp.In.CropX                                                    | `mfxU16`    | `mfxVideoParam.vpp.In.CropX`                                      |               |
| vpp.In.CropY                                                    | `mfxU16`    | `mfxVideoParam.vpp.In.CropY`                                      |               |
| vpp.In.CropW                                                    | `mfxU16`    | `mfxVideoParam.vpp.In.CropW`                                      |               |
| vpp.In.CropH                                                    | `mfxU16`    | `mfxVideoParam.vpp.In.CropH`                                      |               |
| vpp.In.BufferSize                                               | `mfxU64`    | `mfxVideoParam.vpp.In.BufferSize`                                 |               |
| vpp.In.FrameRateExtN                                            | `mfxU32`    | `mfxVideoParam.vpp.In.FrameRateExtN`                              |               |
| vpp.In.FrameRateExtD                                            | `mfxU32`    | `mfxVideoParam.vpp.In.FrameRateExtD`                              |               |
| vpp.In.AspectRatioW                                             | `mfxU16`    | `mfxVideoParam.vpp.In.AspectRatioW`                               |               |
| vpp.In.AspectRatioH                                             | `mfxU16`    | `mfxVideoParam.vpp.In.AspectRatioH`                               |               |
| vpp.In.PicStruct                                                | `mfxU16`    | `mfxVideoParam.vpp.In.PicStruct`                                  |               |
| vpp.In.ChromaFormat                                             | `mfxU16`    | `mfxVideoParam.vpp.In.ChromaFormat`                               |               |
| vpp.Out.ChannelId                                               | `mfxU16`    | `mfxVideoParam.vpp.Out.ChannelId`                                 |               |
| vpp.Out.BitDepthLuma                                            | `mfxU16`    | `mfxVideoParam.vpp.Out.BitDepthLuma`                              |               |
| vpp.Out.BitDepthChroma                                          | `mfxU16`    | `mfxVideoParam.vpp.Out.BitDepthChroma`                            |               |
| vpp.Out.Shift                                                   | `mfxU16`    | `mfxVideoParam.vpp.Out.Shift`                                     |               |
| vpp.Out.FrameId.TemporalId                                      | `mfxU16`    | `mfxVideoParam.vpp.Out.FrameId.TemporalId`                        |               |
| vpp.Out.FrameId.PriorityId                                      | `mfxU16`    | `mfxVideoParam.vpp.Out.FrameId.PriorityId`                        |               |
| vpp.Out.FrameId.DependencyId                                    | `mfxU16`    | `mfxVideoParam.vpp.Out.FrameId.DependencyId`                      |               |
| vpp.Out.FrameId.QualityId                                       | `mfxU16`    | `mfxVideoParam.vpp.Out.FrameId.QualityId`                         |               |
| vpp.Out.FrameId.ViewId                                          | `mfxU16`    | `mfxVideoParam.vpp.Out.FrameId.ViewId`                            |               |
| vpp.Out.FourCC                                                  | `mfxU32`    | `mfxVideoParam.vpp.Out.FourCC`                                    |               |
| vpp.Out.Width                                                   | `mfxU16`    | `mfxVideoParam.vpp.Out.Width`                                     |               |
| vpp.Out.Height                                                  | `mfxU16`    | `mfxVideoParam.vpp.Out.Height`                                    |               |
| vpp.Out.CropX                                                   | `mfxU16`    | `mfxVideoParam.vpp.Out.CropX`                                     |               |
| vpp.Out.CropY                                                   | `mfxU16`    | `mfxVideoParam.vpp.Out.CropY`                                     |               |
| vpp.Out.CropW                                                   | `mfxU16`    | `mfxVideoParam.vpp.Out.CropW`                                     |               |
| vpp.Out.CropH                                                   | `mfxU16`    | `mfxVideoParam.vpp.Out.CropH`                                     |               |
| vpp.Out.BufferSize                                              | `mfxU64`    | `mfxVideoParam.vpp.Out.BufferSize`                                |               |
| vpp.Out.FrameRateExtN                                           | `mfxU32`    | `mfxVideoParam.vpp.Out.FrameRateExtN`                             |               |
| vpp.Out.FrameRateExtD                                           | `mfxU32`    | `mfxVideoParam.vpp.Out.FrameRateExtD`                             |               |
| vpp.Out.AspectRatioW                                            | `mfxU16`    | `mfxVideoParam.vpp.Out.AspectRatioW`                              |               |
| vpp.Out.AspectRatioH                                            | `mfxU16`    | `mfxVideoParam.vpp.Out.AspectRatioH`                              |               |
| vpp.Out.PicStruct                                               | `mfxU16`    | `mfxVideoParam.vpp.Out.PicStruct`                                 |               |
| vpp.Out.ChromaFormat                                            | `mfxU16`    | `mfxVideoParam.vpp.Out.ChromaFormat`                              |               |
| Protected                                                       | `mfxU16`    | `mfxVideoParam.Protected`                                         |               |
| IOPattern                                                       | `mfxU16`    | `mfxVideoParam.IOPattern`                                         |               |
|                                                                 | `mfxU32`    | `mfxVideoParam.ExtParam*.BufferId`                                | pointer       |
|                                                                 | `mfxU32`    | `mfxVideoParam.ExtParam*.BufferSz`                                | pointer       |
| NumExtParam                                                     | `mfxU16`    | `mfxVideoParam.NumExtParam`                                       |               |
| mfxExtAV1BitstreamParam.WriteIVFHeaders                         | `mfxU16`    | `mfxExtAV1BitstreamParam.WriteIVFHeaders`                         |               |
| mfxExtAV1FilmGrainParam.FilmGrainFlags                          | `mfxU16`    | `mfxExtAV1FilmGrainParam.FilmGrainFlags`                          |               |
| mfxExtAV1FilmGrainParam.GrainSeed                               | `mfxU16`    | `mfxExtAV1FilmGrainParam.GrainSeed`                               |               |
| mfxExtAV1FilmGrainParam.RefIdx                                  | `mfxU8`     | `mfxExtAV1FilmGrainParam.RefIdx`                                  |               |
| mfxExtAV1FilmGrainParam.NumYPoints                              | `mfxU8`     | `mfxExtAV1FilmGrainParam.NumYPoints`                              |               |
| mfxExtAV1FilmGrainParam.NumCbPoints                             | `mfxU8`     | `mfxExtAV1FilmGrainParam.NumCbPoints`                             |               |
| mfxExtAV1FilmGrainParam.NumCrPoints                             | `mfxU8`     | `mfxExtAV1FilmGrainParam.NumCrPoints`                             |               |
|                                                                 | `mfxU8`     | `mfxExtAV1FilmGrainParam.PointY[14].Value`                        | array         |
|                                                                 | `mfxU8`     | `mfxExtAV1FilmGrainParam.PointY[14].Scaling`                      | array         |
|                                                                 | `mfxU8`     | `mfxExtAV1FilmGrainParam.PointCb[10].Value`                       | array         |
|                                                                 | `mfxU8`     | `mfxExtAV1FilmGrainParam.PointCb[10].Scaling`                     | array         |
|                                                                 | `mfxU8`     | `mfxExtAV1FilmGrainParam.PointCr[10].Value`                       | array         |
|                                                                 | `mfxU8`     | `mfxExtAV1FilmGrainParam.PointCr[10].Scaling`                     | array         |
| mfxExtAV1FilmGrainParam.GrainScalingMinus8                      | `mfxU8`     | `mfxExtAV1FilmGrainParam.GrainScalingMinus8`                      |               |
| mfxExtAV1FilmGrainParam.ArCoeffLag                              | `mfxU8`     | `mfxExtAV1FilmGrainParam.ArCoeffLag`                              |               |
|                                                                 | `mfxU8`     | `mfxExtAV1FilmGrainParam.ArCoeffsYPlus128[24]`                    | array         |
|                                                                 | `mfxU8`     | `mfxExtAV1FilmGrainParam.ArCoeffsCbPlus128[25]`                   | array         |
|                                                                 | `mfxU8`     | `mfxExtAV1FilmGrainParam.ArCoeffsCrPlus128[25]`                   | array         |
| mfxExtAV1FilmGrainParam.ArCoeffShiftMinus6                      | `mfxU8`     | `mfxExtAV1FilmGrainParam.ArCoeffShiftMinus6`                      |               |
| mfxExtAV1FilmGrainParam.GrainScaleShift                         | `mfxU8`     | `mfxExtAV1FilmGrainParam.GrainScaleShift`                         |               |
| mfxExtAV1FilmGrainParam.CbMult                                  | `mfxU8`     | `mfxExtAV1FilmGrainParam.CbMult`                                  |               |
| mfxExtAV1FilmGrainParam.CbLumaMult                              | `mfxU8`     | `mfxExtAV1FilmGrainParam.CbLumaMult`                              |               |
| mfxExtAV1FilmGrainParam.CbOffset                                | `mfxU16`    | `mfxExtAV1FilmGrainParam.CbOffset`                                |               |
| mfxExtAV1FilmGrainParam.CrMult                                  | `mfxU8`     | `mfxExtAV1FilmGrainParam.CrMult`                                  |               |
| mfxExtAV1FilmGrainParam.CrLumaMult                              | `mfxU8`     | `mfxExtAV1FilmGrainParam.CrLumaMult`                              |               |
| mfxExtAV1FilmGrainParam.CrOffset                                | `mfxU16`    | `mfxExtAV1FilmGrainParam.CrOffset`                                |               |
| mfxExtAV1ResolutionParam.FrameWidth                             | `mfxU32`    | `mfxExtAV1ResolutionParam.FrameWidth`                             |               |
| mfxExtAV1ResolutionParam.FrameHeight                            | `mfxU32`    | `mfxExtAV1ResolutionParam.FrameHeight`                            |               |
| mfxExtAV1Segmentation.NumSegments                               | `mfxU8`     | `mfxExtAV1Segmentation.NumSegments`                               |               |
|                                                                 | `mfxU16`    | `mfxExtAV1Segmentation.Segment[8].FeatureEnabled`                 | array         |
|                                                                 | `mfxI16`    | `mfxExtAV1Segmentation.Segment[8].AltQIndex`                      | array         |
| mfxExtAV1Segmentation.SegmentIdBlockSize                        | `mfxU16`    | `mfxExtAV1Segmentation.SegmentIdBlockSize`                        |               |
| mfxExtAV1Segmentation.NumSegmentIdAlloc                         | `mfxU32`    | `mfxExtAV1Segmentation.NumSegmentIdAlloc`                         |               |
|                                                                 | `mfxU8`     | `mfxExtAV1Segmentation.SegmentIds*`                               | pointer       |
| mfxExtAV1TileParam.NumTileRows                                  | `mfxU16`    | `mfxExtAV1TileParam.NumTileRows`                                  |               |
| mfxExtAV1TileParam.NumTileColumns                               | `mfxU16`    | `mfxExtAV1TileParam.NumTileColumns`                               |               |
| mfxExtAV1TileParam.NumTileGroups                                | `mfxU16`    | `mfxExtAV1TileParam.NumTileGroups`                                |               |
| mfxExtAVCEncodedFrameInfo.FrameOrder                            | `mfxU32`    | `mfxExtAVCEncodedFrameInfo.FrameOrder`                            |               |
| mfxExtAVCEncodedFrameInfo.PicStruct                             | `mfxU16`    | `mfxExtAVCEncodedFrameInfo.PicStruct`                             |               |
| mfxExtAVCEncodedFrameInfo.LongTermIdx                           | `mfxU16`    | `mfxExtAVCEncodedFrameInfo.LongTermIdx`                           |               |
| mfxExtAVCEncodedFrameInfo.MAD                                   | `mfxU32`    | `mfxExtAVCEncodedFrameInfo.MAD`                                   |               |
| mfxExtAVCEncodedFrameInfo.BRCPanicMode                          | `mfxU16`    | `mfxExtAVCEncodedFrameInfo.BRCPanicMode`                          |               |
| mfxExtAVCEncodedFrameInfo.QP                                    | `mfxU16`    | `mfxExtAVCEncodedFrameInfo.QP`                                    |               |
| mfxExtAVCEncodedFrameInfo.SecondFieldOffset                     | `mfxU32`    | `mfxExtAVCEncodedFrameInfo.SecondFieldOffset`                     |               |
|                                                                 | `mfxU32`    | `mfxExtAVCEncodedFrameInfo.UsedRefListL0[32].FrameOrder`          | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCEncodedFrameInfo.UsedRefListL0[32].PicStruct`           | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCEncodedFrameInfo.UsedRefListL0[32].LongTermIdx`         | array         |
|                                                                 | `mfxU32`    | `mfxExtAVCEncodedFrameInfo.UsedRefListL1[32].FrameOrder`          | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCEncodedFrameInfo.UsedRefListL1[32].PicStruct`           | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCEncodedFrameInfo.UsedRefListL1[32].LongTermIdx`         | array         |
| mfxExtAVCRefListCtrl.NumRefIdxL0Active                          | `mfxU16`    | `mfxExtAVCRefListCtrl.NumRefIdxL0Active`                          |               |
| mfxExtAVCRefListCtrl.NumRefIdxL1Active                          | `mfxU16`    | `mfxExtAVCRefListCtrl.NumRefIdxL1Active`                          |               |
|                                                                 | `mfxU32`    | `mfxExtAVCRefListCtrl.PreferredRefList[32].FrameOrder`            | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCRefListCtrl.PreferredRefList[32].PicStruct`             | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCRefListCtrl.PreferredRefList[32].ViewId`                | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCRefListCtrl.PreferredRefList[32].LongTermIdx`           | array         |
|                                                                 | `mfxU32`    | `mfxExtAVCRefListCtrl.RejectedRefList[16].FrameOrder`             | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCRefListCtrl.RejectedRefList[16].PicStruct`              | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCRefListCtrl.RejectedRefList[16].ViewId`                 | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCRefListCtrl.RejectedRefList[16].LongTermIdx`            | array         |
|                                                                 | `mfxU32`    | `mfxExtAVCRefListCtrl.LongTermRefList[16].FrameOrder`             | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCRefListCtrl.LongTermRefList[16].PicStruct`              | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCRefListCtrl.LongTermRefList[16].ViewId`                 | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCRefListCtrl.LongTermRefList[16].LongTermIdx`            | array         |
| mfxExtAVCRefListCtrl.ApplyLongTermIdx                           | `mfxU16`    | `mfxExtAVCRefListCtrl.ApplyLongTermIdx`                           |               |
| mfxExtAVCRefLists.NumRefIdxL0Active                             | `mfxU16`    | `mfxExtAVCRefLists.NumRefIdxL0Active`                             |               |
| mfxExtAVCRefLists.NumRefIdxL1Active                             | `mfxU16`    | `mfxExtAVCRefLists.NumRefIdxL1Active`                             |               |
|                                                                 | `mfxU32`    | `mfxExtAVCRefLists.RefPicList0[32].FrameOrder`                    | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCRefLists.RefPicList0[32].PicStruct`                     | array         |
|                                                                 | `mfxU32`    | `mfxExtAVCRefLists.RefPicList1[32].FrameOrder`                    | array         |
|                                                                 | `mfxU16`    | `mfxExtAVCRefLists.RefPicList1[32].PicStruct`                     | array         |
| mfxExtAVCRoundingOffset.EnableRoundingIntra                     | `mfxU16`    | `mfxExtAVCRoundingOffset.EnableRoundingIntra`                     |               |
| mfxExtAVCRoundingOffset.RoundingOffsetIntra                     | `mfxU16`    | `mfxExtAVCRoundingOffset.RoundingOffsetIntra`                     |               |
| mfxExtAVCRoundingOffset.EnableRoundingInter                     | `mfxU16`    | `mfxExtAVCRoundingOffset.EnableRoundingInter`                     |               |
| mfxExtAVCRoundingOffset.RoundingOffsetInter                     | `mfxU16`    | `mfxExtAVCRoundingOffset.RoundingOffsetInter`                     |               |
| mfxExtAvcTemporalLayers.BaseLayerPID                            | `mfxU16`    | `mfxExtAvcTemporalLayers.BaseLayerPID`                            |               |
|                                                                 | `mfxU16`    | `mfxExtAvcTemporalLayers.Layer[8].Scale`                          | array         |
| mfxExtChromaLocInfo.ChromaLocInfoPresentFlag                    | `mfxU16`    | `mfxExtChromaLocInfo.ChromaLocInfoPresentFlag`                    |               |
| mfxExtChromaLocInfo.ChromaSampleLocTypeTopField                 | `mfxU16`    | `mfxExtChromaLocInfo.ChromaSampleLocTypeTopField`                 |               |
| mfxExtChromaLocInfo.ChromaSampleLocTypeBottomField              | `mfxU16`    | `mfxExtChromaLocInfo.ChromaSampleLocTypeBottomField`              |               |
| mfxExtCodingOption.RateDistortionOpt                            | `mfxU16`    | `mfxExtCodingOption.RateDistortionOpt`                            |               |
| mfxExtCodingOption.MECostType                                   | `mfxU16`    | `mfxExtCodingOption.MECostType`                                   |               |
| mfxExtCodingOption.MESearchType                                 | `mfxU16`    | `mfxExtCodingOption.MESearchType`                                 |               |
| mfxExtCodingOption.MVSearchWindow.x                             | `mfxI16`    | `mfxExtCodingOption.MVSearchWindow.x`                             |               |
| mfxExtCodingOption.MVSearchWindow.y                             | `mfxI16`    | `mfxExtCodingOption.MVSearchWindow.y`                             |               |
| mfxExtCodingOption.EndOfSequence                                | `mfxU16`    | `mfxExtCodingOption.EndOfSequence`                                |               |
| mfxExtCodingOption.FramePicture                                 | `mfxU16`    | `mfxExtCodingOption.FramePicture`                                 |               |
| mfxExtCodingOption.CAVLC                                        | `mfxU16`    | `mfxExtCodingOption.CAVLC`                                        |               |
| mfxExtCodingOption.RecoveryPointSEI                             | `mfxU16`    | `mfxExtCodingOption.RecoveryPointSEI`                             |               |
| mfxExtCodingOption.ViewOutput                                   | `mfxU16`    | `mfxExtCodingOption.ViewOutput`                                   |               |
| mfxExtCodingOption.NalHrdConformance                            | `mfxU16`    | `mfxExtCodingOption.NalHrdConformance`                            |               |
| mfxExtCodingOption.SingleSeiNalUnit                             | `mfxU16`    | `mfxExtCodingOption.SingleSeiNalUnit`                             |               |
| mfxExtCodingOption.VuiVclHrdParameters                          | `mfxU16`    | `mfxExtCodingOption.VuiVclHrdParameters`                          |               |
| mfxExtCodingOption.RefPicListReordering                         | `mfxU16`    | `mfxExtCodingOption.RefPicListReordering`                         |               |
| mfxExtCodingOption.ResetRefList                                 | `mfxU16`    | `mfxExtCodingOption.ResetRefList`                                 |               |
| mfxExtCodingOption.RefPicMarkRep                                | `mfxU16`    | `mfxExtCodingOption.RefPicMarkRep`                                |               |
| mfxExtCodingOption.FieldOutput                                  | `mfxU16`    | `mfxExtCodingOption.FieldOutput`                                  |               |
| mfxExtCodingOption.IntraPredBlockSize                           | `mfxU16`    | `mfxExtCodingOption.IntraPredBlockSize`                           |               |
| mfxExtCodingOption.InterPredBlockSize                           | `mfxU16`    | `mfxExtCodingOption.InterPredBlockSize`                           |               |
| mfxExtCodingOption.MVPrecision                                  | `mfxU16`    | `mfxExtCodingOption.MVPrecision`                                  |               |
| mfxExtCodingOption.MaxDecFrameBuffering                         | `mfxU16`    | `mfxExtCodingOption.MaxDecFrameBuffering`                         |               |
| mfxExtCodingOption.AUDelimiter                                  | `mfxU16`    | `mfxExtCodingOption.AUDelimiter`                                  |               |
| mfxExtCodingOption.EndOfStream                                  | `mfxU16`    | `mfxExtCodingOption.EndOfStream`                                  |               |
| mfxExtCodingOption.PicTimingSEI                                 | `mfxU16`    | `mfxExtCodingOption.PicTimingSEI`                                 |               |
| mfxExtCodingOption.VuiNalHrdParameters                          | `mfxU16`    | `mfxExtCodingOption.VuiNalHrdParameters`                          |               |
| mfxExtCodingOption2.IntRefType                                  | `mfxU16`    | `mfxExtCodingOption2.IntRefType`                                  |               |
| mfxExtCodingOption2.IntRefCycleSize                             | `mfxU16`    | `mfxExtCodingOption2.IntRefCycleSize`                             |               |
| mfxExtCodingOption2.IntRefQPDelta                               | `mfxI16`    | `mfxExtCodingOption2.IntRefQPDelta`                               |               |
| mfxExtCodingOption2.MaxFrameSize                                | `mfxU32`    | `mfxExtCodingOption2.MaxFrameSize`                                |               |
| mfxExtCodingOption2.MaxSliceSize                                | `mfxU32`    | `mfxExtCodingOption2.MaxSliceSize`                                |               |
| mfxExtCodingOption2.BitrateLimit                                | `mfxU16`    | `mfxExtCodingOption2.BitrateLimit`                                |               |
| mfxExtCodingOption2.MBBRC                                       | `mfxU16`    | `mfxExtCodingOption2.MBBRC`                                       |               |
| mfxExtCodingOption2.ExtBRC                                      | `mfxU16`    | `mfxExtCodingOption2.ExtBRC`                                      |               |
| mfxExtCodingOption2.LookAheadDepth                              | `mfxU16`    | `mfxExtCodingOption2.LookAheadDepth`                              |               |
| mfxExtCodingOption2.Trellis                                     | `mfxU16`    | `mfxExtCodingOption2.Trellis`                                     |               |
| mfxExtCodingOption2.RepeatPPS                                   | `mfxU16`    | `mfxExtCodingOption2.RepeatPPS`                                   |               |
| mfxExtCodingOption2.BRefType                                    | `mfxU16`    | `mfxExtCodingOption2.BRefType`                                    |               |
| mfxExtCodingOption2.AdaptiveI                                   | `mfxU16`    | `mfxExtCodingOption2.AdaptiveI`                                   |               |
| mfxExtCodingOption2.AdaptiveB                                   | `mfxU16`    | `mfxExtCodingOption2.AdaptiveB`                                   |               |
| mfxExtCodingOption2.LookAheadDS                                 | `mfxU16`    | `mfxExtCodingOption2.LookAheadDS`                                 |               |
| mfxExtCodingOption2.NumMbPerSlice                               | `mfxU16`    | `mfxExtCodingOption2.NumMbPerSlice`                               |               |
| mfxExtCodingOption2.SkipFrame                                   | `mfxU16`    | `mfxExtCodingOption2.SkipFrame`                                   |               |
| mfxExtCodingOption2.MinQPI                                      | `mfxU8`     | `mfxExtCodingOption2.MinQPI`                                      |               |
| mfxExtCodingOption2.MaxQPI                                      | `mfxU8`     | `mfxExtCodingOption2.MaxQPI`                                      |               |
| mfxExtCodingOption2.MinQPP                                      | `mfxU8`     | `mfxExtCodingOption2.MinQPP`                                      |               |
| mfxExtCodingOption2.MaxQPP                                      | `mfxU8`     | `mfxExtCodingOption2.MaxQPP`                                      |               |
| mfxExtCodingOption2.MinQPB                                      | `mfxU8`     | `mfxExtCodingOption2.MinQPB`                                      |               |
| mfxExtCodingOption2.MaxQPB                                      | `mfxU8`     | `mfxExtCodingOption2.MaxQPB`                                      |               |
| mfxExtCodingOption2.FixedFrameRate                              | `mfxU16`    | `mfxExtCodingOption2.FixedFrameRate`                              |               |
| mfxExtCodingOption2.DisableDeblockingIdc                        | `mfxU16`    | `mfxExtCodingOption2.DisableDeblockingIdc`                        |               |
| mfxExtCodingOption2.DisableVUI                                  | `mfxU16`    | `mfxExtCodingOption2.DisableVUI`                                  |               |
| mfxExtCodingOption2.BufferingPeriodSEI                          | `mfxU16`    | `mfxExtCodingOption2.BufferingPeriodSEI`                          |               |
| mfxExtCodingOption2.EnableMAD                                   | `mfxU16`    | `mfxExtCodingOption2.EnableMAD`                                   |               |
| mfxExtCodingOption2.UseRawRef                                   | `mfxU16`    | `mfxExtCodingOption2.UseRawRef`                                   |               |
| mfxExtCodingOption3.NumSliceI                                   | `mfxU16`    | `mfxExtCodingOption3.NumSliceI`                                   |               |
| mfxExtCodingOption3.NumSliceP                                   | `mfxU16`    | `mfxExtCodingOption3.NumSliceP`                                   |               |
| mfxExtCodingOption3.NumSliceB                                   | `mfxU16`    | `mfxExtCodingOption3.NumSliceB`                                   |               |
| mfxExtCodingOption3.WinBRCMaxAvgKbps                            | `mfxU16`    | `mfxExtCodingOption3.WinBRCMaxAvgKbps`                            |               |
| mfxExtCodingOption3.WinBRCSize                                  | `mfxU16`    | `mfxExtCodingOption3.WinBRCSize`                                  |               |
| mfxExtCodingOption3.QVBRQuality                                 | `mfxU16`    | `mfxExtCodingOption3.QVBRQuality`                                 |               |
| mfxExtCodingOption3.EnableMBQP                                  | `mfxU16`    | `mfxExtCodingOption3.EnableMBQP`                                  |               |
| mfxExtCodingOption3.IntRefCycleDist                             | `mfxU16`    | `mfxExtCodingOption3.IntRefCycleDist`                             |               |
| mfxExtCodingOption3.DirectBiasAdjustment                        | `mfxU16`    | `mfxExtCodingOption3.DirectBiasAdjustment`                        |               |
| mfxExtCodingOption3.GlobalMotionBiasAdjustment                  | `mfxU16`    | `mfxExtCodingOption3.GlobalMotionBiasAdjustment`                  |               |
| mfxExtCodingOption3.MVCostScalingFactor                         | `mfxU16`    | `mfxExtCodingOption3.MVCostScalingFactor`                         |               |
| mfxExtCodingOption3.MBDisableSkipMap                            | `mfxU16`    | `mfxExtCodingOption3.MBDisableSkipMap`                            |               |
| mfxExtCodingOption3.WeightedPred                                | `mfxU16`    | `mfxExtCodingOption3.WeightedPred`                                |               |
| mfxExtCodingOption3.WeightedBiPred                              | `mfxU16`    | `mfxExtCodingOption3.WeightedBiPred`                              |               |
| mfxExtCodingOption3.AspectRatioInfoPresent                      | `mfxU16`    | `mfxExtCodingOption3.AspectRatioInfoPresent`                      |               |
| mfxExtCodingOption3.OverscanInfoPresent                         | `mfxU16`    | `mfxExtCodingOption3.OverscanInfoPresent`                         |               |
| mfxExtCodingOption3.OverscanAppropriate                         | `mfxU16`    | `mfxExtCodingOption3.OverscanAppropriate`                         |               |
| mfxExtCodingOption3.TimingInfoPresent                           | `mfxU16`    | `mfxExtCodingOption3.TimingInfoPresent`                           |               |
| mfxExtCodingOption3.BitstreamRestriction                        | `mfxU16`    | `mfxExtCodingOption3.BitstreamRestriction`                        |               |
| mfxExtCodingOption3.LowDelayHrd                                 | `mfxU16`    | `mfxExtCodingOption3.LowDelayHrd`                                 |               |
| mfxExtCodingOption3.MotionVectorsOverPicBoundaries              | `mfxU16`    | `mfxExtCodingOption3.MotionVectorsOverPicBoundaries`              |               |
| mfxExtCodingOption3.ScenarioInfo                                | `mfxU16`    | `mfxExtCodingOption3.ScenarioInfo`                                |               |
| mfxExtCodingOption3.ContentInfo                                 | `mfxU16`    | `mfxExtCodingOption3.ContentInfo`                                 |               |
| mfxExtCodingOption3.PRefType                                    | `mfxU16`    | `mfxExtCodingOption3.PRefType`                                    |               |
| mfxExtCodingOption3.FadeDetection                               | `mfxU16`    | `mfxExtCodingOption3.FadeDetection`                               |               |
| mfxExtCodingOption3.GPB                                         | `mfxU16`    | `mfxExtCodingOption3.GPB`                                         |               |
| mfxExtCodingOption3.MaxFrameSizeI                               | `mfxU32`    | `mfxExtCodingOption3.MaxFrameSizeI`                               |               |
| mfxExtCodingOption3.MaxFrameSizeP                               | `mfxU32`    | `mfxExtCodingOption3.MaxFrameSizeP`                               |               |
| mfxExtCodingOption3.EnableQPOffset                              | `mfxU16`    | `mfxExtCodingOption3.EnableQPOffset`                              |               |
|                                                                 | `mfxI16`    | `mfxExtCodingOption3.QPOffset[8]`                                 | array         |
|                                                                 | `mfxU16`    | `mfxExtCodingOption3.NumRefActiveP[8]`                            | array         |
|                                                                 | `mfxU16`    | `mfxExtCodingOption3.NumRefActiveBL0[8]`                          | array         |
|                                                                 | `mfxU16`    | `mfxExtCodingOption3.NumRefActiveBL1[8]`                          | array         |
| mfxExtCodingOption3.TransformSkip                               | `mfxU16`    | `mfxExtCodingOption3.TransformSkip`                               |               |
| mfxExtCodingOption3.TargetChromaFormatPlus1                     | `mfxU16`    | `mfxExtCodingOption3.TargetChromaFormatPlus1`                     |               |
| mfxExtCodingOption3.TargetBitDepthLuma                          | `mfxU16`    | `mfxExtCodingOption3.TargetBitDepthLuma`                          |               |
| mfxExtCodingOption3.TargetBitDepthChroma                        | `mfxU16`    | `mfxExtCodingOption3.TargetBitDepthChroma`                        |               |
| mfxExtCodingOption3.BRCPanicMode                                | `mfxU16`    | `mfxExtCodingOption3.BRCPanicMode`                                |               |
| mfxExtCodingOption3.LowDelayBRC                                 | `mfxU16`    | `mfxExtCodingOption3.LowDelayBRC`                                 |               |
| mfxExtCodingOption3.EnableMBForceIntra                          | `mfxU16`    | `mfxExtCodingOption3.EnableMBForceIntra`                          |               |
| mfxExtCodingOption3.AdaptiveMaxFrameSize                        | `mfxU16`    | `mfxExtCodingOption3.AdaptiveMaxFrameSize`                        |               |
| mfxExtCodingOption3.RepartitionCheckEnable                      | `mfxU16`    | `mfxExtCodingOption3.RepartitionCheckEnable`                      |               |
| mfxExtCodingOption3.EncodedUnitsInfo                            | `mfxU16`    | `mfxExtCodingOption3.EncodedUnitsInfo`                            |               |
| mfxExtCodingOption3.EnableNalUnitType                           | `mfxU16`    | `mfxExtCodingOption3.EnableNalUnitType`                           |               |
| mfxExtCodingOption3.ExtBrcAdaptiveLTR                           | `mfxU16`    | `mfxExtCodingOption3.ExtBrcAdaptiveLTR`                           |               |
| mfxExtCodingOption3.AdaptiveLTR                                 | `mfxU16`    | `mfxExtCodingOption3.AdaptiveLTR`                                 |               |
| mfxExtCodingOption3.AdaptiveCQM                                 | `mfxU16`    | `mfxExtCodingOption3.AdaptiveCQM`                                 |               |
| mfxExtCodingOption3.AdaptiveRef                                 | `mfxU16`    | `mfxExtCodingOption3.AdaptiveRef`                                 |               |
|                                                                 | `mfxU8`     | `mfxExtCodingOptionSPSPPS.SPSBuffer*`                             | pointer       |
|                                                                 | `mfxU8`     | `mfxExtCodingOptionSPSPPS.PPSBuffer*`                             | pointer       |
| mfxExtCodingOptionSPSPPS.SPSBufSize                             | `mfxU16`    | `mfxExtCodingOptionSPSPPS.SPSBufSize`                             |               |
| mfxExtCodingOptionSPSPPS.PPSBufSize                             | `mfxU16`    | `mfxExtCodingOptionSPSPPS.PPSBufSize`                             |               |
| mfxExtCodingOptionSPSPPS.SPSId                                  | `mfxU16`    | `mfxExtCodingOptionSPSPPS.SPSId`                                  |               |
| mfxExtCodingOptionSPSPPS.PPSId                                  | `mfxU16`    | `mfxExtCodingOptionSPSPPS.PPSId`                                  |               |
|                                                                 | `mfxU8`     | `mfxExtCodingOptionVPS.VPSBuffer*`                                | pointer       |
| mfxExtCodingOptionVPS.VPSBufSize                                | `mfxU16`    | `mfxExtCodingOptionVPS.VPSBufSize`                                |               |
| mfxExtCodingOptionVPS.VPSId                                     | `mfxU16`    | `mfxExtCodingOptionVPS.VPSId`                                     |               |
| mfxExtColorConversion.ChromaSiting                              | `mfxU16`    | `mfxExtColorConversion.ChromaSiting`                              |               |
| mfxExtContentLightLevelInfo.InsertPayloadToggle                 | `mfxU16`    | `mfxExtContentLightLevelInfo.InsertPayloadToggle`                 |               |
| mfxExtContentLightLevelInfo.MaxContentLightLevel                | `mfxU16`    | `mfxExtContentLightLevelInfo.MaxContentLightLevel`                |               |
| mfxExtContentLightLevelInfo.MaxPicAverageLightLevel             | `mfxU16`    | `mfxExtContentLightLevelInfo.MaxPicAverageLightLevel`             |               |
| mfxExtDecVideoProcessing.In.CropX                               | `mfxU16`    | `mfxExtDecVideoProcessing.In.CropX`                               |               |
| mfxExtDecVideoProcessing.In.CropY                               | `mfxU16`    | `mfxExtDecVideoProcessing.In.CropY`                               |               |
| mfxExtDecVideoProcessing.In.CropW                               | `mfxU16`    | `mfxExtDecVideoProcessing.In.CropW`                               |               |
| mfxExtDecVideoProcessing.In.CropH                               | `mfxU16`    | `mfxExtDecVideoProcessing.In.CropH`                               |               |
| mfxExtDecVideoProcessing.Out.FourCC                             | `mfxU32`    | `mfxExtDecVideoProcessing.Out.FourCC`                             |               |
| mfxExtDecVideoProcessing.Out.ChromaFormat                       | `mfxU16`    | `mfxExtDecVideoProcessing.Out.ChromaFormat`                       |               |
| mfxExtDecVideoProcessing.Out.Width                              | `mfxU16`    | `mfxExtDecVideoProcessing.Out.Width`                              |               |
| mfxExtDecVideoProcessing.Out.Height                             | `mfxU16`    | `mfxExtDecVideoProcessing.Out.Height`                             |               |
| mfxExtDecVideoProcessing.Out.CropX                              | `mfxU16`    | `mfxExtDecVideoProcessing.Out.CropX`                              |               |
| mfxExtDecVideoProcessing.Out.CropY                              | `mfxU16`    | `mfxExtDecVideoProcessing.Out.CropY`                              |               |
| mfxExtDecVideoProcessing.Out.CropW                              | `mfxU16`    | `mfxExtDecVideoProcessing.Out.CropW`                              |               |
| mfxExtDecVideoProcessing.Out.CropH                              | `mfxU16`    | `mfxExtDecVideoProcessing.Out.CropH`                              |               |
| mfxExtDecodeErrorReport.ErrorTypes                              | `mfxU32`    | `mfxExtDecodeErrorReport.ErrorTypes`                              |               |
| mfxExtDecodedFrameInfo.FrameType                                | `mfxU16`    | `mfxExtDecodedFrameInfo.FrameType`                                |               |
|                                                                 | `mfxChar`   | `mfxExtDeviceAffinityMask.DeviceID[128]`                          | array         |
| mfxExtDeviceAffinityMask.NumSubDevices                          | `mfxU32`    | `mfxExtDeviceAffinityMask.NumSubDevices`                          |               |
|                                                                 | `mfxU8`     | `mfxExtDeviceAffinityMask.Mask*`                                  | pointer       |
| mfxExtDirtyRect.NumRect                                         | `mfxU16`    | `mfxExtDirtyRect.NumRect`                                         |               |
|                                                                 | `mfxU32`    | `mfxExtDirtyRect.Rect[256].Left`                                  | array         |
|                                                                 | `mfxU32`    | `mfxExtDirtyRect.Rect[256].Top`                                   | array         |
|                                                                 | `mfxU32`    | `mfxExtDirtyRect.Rect[256].Right`                                 | array         |
|                                                                 | `mfxU32`    | `mfxExtDirtyRect.Rect[256].Bottom`                                | array         |
| mfxExtEncodedSlicesInfo.SliceSizeOverflow                       | `mfxU16`    | `mfxExtEncodedSlicesInfo.SliceSizeOverflow`                       |               |
| mfxExtEncodedSlicesInfo.NumSliceNonCopliant                     | `mfxU16`    | `mfxExtEncodedSlicesInfo.NumSliceNonCopliant`                     |               |
| mfxExtEncodedSlicesInfo.NumEncodedSlice                         | `mfxU16`    | `mfxExtEncodedSlicesInfo.NumEncodedSlice`                         |               |
| mfxExtEncodedSlicesInfo.NumSliceSizeAlloc                       | `mfxU16`    | `mfxExtEncodedSlicesInfo.NumSliceSizeAlloc`                       |               |
|                                                                 | `mfxU16`    | `mfxExtEncodedSlicesInfo.SliceSize*`                              | pointer       |
|                                                                 | `mfxU16`    | `mfxExtEncodedUnitsInfo.UnitInfo*.Type`                           | pointer       |
|                                                                 | `mfxU32`    | `mfxExtEncodedUnitsInfo.UnitInfo*.Offset`                         | pointer       |
|                                                                 | `mfxU32`    | `mfxExtEncodedUnitsInfo.UnitInfo*.Size`                           | pointer       |
| mfxExtEncodedUnitsInfo.NumUnitsAlloc                            | `mfxU16`    | `mfxExtEncodedUnitsInfo.NumUnitsAlloc`                            |               |
| mfxExtEncodedUnitsInfo.NumUnitsEncoded                          | `mfxU16`    | `mfxExtEncodedUnitsInfo.NumUnitsEncoded`                          |               |
| mfxExtEncoderCapability.MBPerSec                                | `mfxU32`    | `mfxExtEncoderCapability.MBPerSec`                                |               |
| mfxExtEncoderIPCMArea.NumArea                                   | `mfxU16`    | `mfxExtEncoderIPCMArea.NumArea`                                   |               |
|                                                                 | `mfxU32`    | `mfxExtEncoderIPCMArea.Areas*.Left`                               | pointer       |
|                                                                 | `mfxU32`    | `mfxExtEncoderIPCMArea.Areas*.Top`                                | pointer       |
|                                                                 | `mfxU32`    | `mfxExtEncoderIPCMArea.Areas*.Right`                              | pointer       |
|                                                                 | `mfxU32`    | `mfxExtEncoderIPCMArea.Areas*.Bottom`                             | pointer       |
| mfxExtEncoderROI.NumROI                                         | `mfxU16`    | `mfxExtEncoderROI.NumROI`                                         |               |
| mfxExtEncoderROI.ROIMode                                        | `mfxU16`    | `mfxExtEncoderROI.ROIMode`                                        |               |
|                                                                 | `mfxU32`    | `mfxExtEncoderROI.ROI[256].Left`                                  | array         |
|                                                                 | `mfxU32`    | `mfxExtEncoderROI.ROI[256].Top`                                   | array         |
|                                                                 | `mfxU32`    | `mfxExtEncoderROI.ROI[256].Right`                                 | array         |
|                                                                 | `mfxU32`    | `mfxExtEncoderROI.ROI[256].Bottom`                                | array         |
|                                                                 | `mfxI16`    | `mfxExtEncoderROI.ROI[256].Priority`                              | array         |
|                                                                 | `mfxI16`    | `mfxExtEncoderROI.ROI[256].DeltaQP`                               | array         |
| mfxExtEncoderResetOption.StartNewSequence                       | `mfxU16`    | `mfxExtEncoderResetOption.StartNewSequence`                       |               |
| mfxExtHEVCParam.PicWidthInLumaSamples                           | `mfxU16`    | `mfxExtHEVCParam.PicWidthInLumaSamples`                           |               |
| mfxExtHEVCParam.PicHeightInLumaSamples                          | `mfxU16`    | `mfxExtHEVCParam.PicHeightInLumaSamples`                          |               |
| mfxExtHEVCParam.GeneralConstraintFlags                          | `mfxU64`    | `mfxExtHEVCParam.GeneralConstraintFlags`                          |               |
| mfxExtHEVCParam.SampleAdaptiveOffset                            | `mfxU16`    | `mfxExtHEVCParam.SampleAdaptiveOffset`                            |               |
| mfxExtHEVCParam.LCUSize                                         | `mfxU16`    | `mfxExtHEVCParam.LCUSize`                                         |               |
| mfxExtHEVCRegion.RegionId                                       | `mfxU32`    | `mfxExtHEVCRegion.RegionId`                                       |               |
| mfxExtHEVCRegion.RegionType                                     | `mfxU16`    | `mfxExtHEVCRegion.RegionType`                                     |               |
| mfxExtHEVCRegion.RegionEncoding                                 | `mfxU16`    | `mfxExtHEVCRegion.RegionEncoding`                                 |               |
| mfxExtHEVCTiles.NumTileRows                                     | `mfxU16`    | `mfxExtHEVCTiles.NumTileRows`                                     |               |
| mfxExtHEVCTiles.NumTileColumns                                  | `mfxU16`    | `mfxExtHEVCTiles.NumTileColumns`                                  |               |
| mfxExtHyperModeParam.Mode                                       | `int`       | `mfxExtHyperModeParam.Mode`                                       |               |
| mfxExtInCrops.Crops.Left                                        | `mfxU16`    | `mfxExtInCrops.Crops.Left`                                        |               |
| mfxExtInCrops.Crops.Top                                         | `mfxU16`    | `mfxExtInCrops.Crops.Top`                                         |               |
| mfxExtInCrops.Crops.Right                                       | `mfxU16`    | `mfxExtInCrops.Crops.Right`                                       |               |
| mfxExtInCrops.Crops.Bottom                                      | `mfxU16`    | `mfxExtInCrops.Crops.Bottom`                                      |               |
| mfxExtInsertHeaders.SPS                                         | `mfxU16`    | `mfxExtInsertHeaders.SPS`                                         |               |
| mfxExtInsertHeaders.PPS                                         | `mfxU16`    | `mfxExtInsertHeaders.PPS`                                         |               |
| mfxExtMBDisableSkipMap.MapSize                                  | `mfxU32`    | `mfxExtMBDisableSkipMap.MapSize`                                  |               |
|                                                                 | `mfxU8`     | `mfxExtMBDisableSkipMap.Map*`                                     | pointer       |
| mfxExtMBForceIntra.MapSize                                      | `mfxU32`    | `mfxExtMBForceIntra.MapSize`                                      |               |
|                                                                 | `mfxU8`     | `mfxExtMBForceIntra.Map*`                                         | pointer       |
| mfxExtMBQP.Mode                                                 | `mfxU16`    | `mfxExtMBQP.Mode`                                                 |               |
| mfxExtMBQP.BlockSize                                            | `mfxU16`    | `mfxExtMBQP.BlockSize`                                            |               |
| mfxExtMBQP.NumQPAlloc                                           | `mfxU32`    | `mfxExtMBQP.NumQPAlloc`                                           |               |
|                                                                 | `mfxU8`     | `mfxExtMBQP.QP*`                                                  | pointer       |
|                                                                 | `mfxI8`     | `mfxExtMBQP.DeltaQP*`                                             | pointer       |
|                                                                 | `mfxU8`     | `mfxExtMBQP.QPmode*.QP`                                           | pointer       |
|                                                                 | `mfxI8`     | `mfxExtMBQP.QPmode*.DeltaQP`                                      | pointer       |
|                                                                 | `mfxU16`    | `mfxExtMBQP.QPmode*.Mode`                                         | pointer       |
| mfxExtMVOverPicBoundaries.StickTop                              | `mfxU16`    | `mfxExtMVOverPicBoundaries.StickTop`                              |               |
| mfxExtMVOverPicBoundaries.StickBottom                           | `mfxU16`    | `mfxExtMVOverPicBoundaries.StickBottom`                           |               |
| mfxExtMVOverPicBoundaries.StickLeft                             | `mfxU16`    | `mfxExtMVOverPicBoundaries.StickLeft`                             |               |
| mfxExtMVOverPicBoundaries.StickRight                            | `mfxU16`    | `mfxExtMVOverPicBoundaries.StickRight`                            |               |
| mfxExtMasteringDisplayColourVolume.InsertPayloadToggle          | `mfxU16`    | `mfxExtMasteringDisplayColourVolume.InsertPayloadToggle`          |               |
|                                                                 | `mfxU16`    | `mfxExtMasteringDisplayColourVolume.DisplayPrimariesX[3]`         | array         |
|                                                                 | `mfxU16`    | `mfxExtMasteringDisplayColourVolume.DisplayPrimariesY[3]`         | array         |
| mfxExtMasteringDisplayColourVolume.WhitePointX                  | `mfxU16`    | `mfxExtMasteringDisplayColourVolume.WhitePointX`                  |               |
| mfxExtMasteringDisplayColourVolume.WhitePointY                  | `mfxU16`    | `mfxExtMasteringDisplayColourVolume.WhitePointY`                  |               |
| mfxExtMasteringDisplayColourVolume.MaxDisplayMasteringLuminance | `mfxU32`    | `mfxExtMasteringDisplayColourVolume.MaxDisplayMasteringLuminance` |               |
| mfxExtMasteringDisplayColourVolume.MinDisplayMasteringLuminance | `mfxU32`    | `mfxExtMasteringDisplayColourVolume.MinDisplayMasteringLuminance` |               |
| mfxExtMoveRect.NumRect                                          | `mfxU16`    | `mfxExtMoveRect.NumRect`                                          |               |
|                                                                 | `mfxU32`    | `mfxExtMoveRect.Rect[256].DestLeft`                               | array         |
|                                                                 | `mfxU32`    | `mfxExtMoveRect.Rect[256].DestTop`                                | array         |
|                                                                 | `mfxU32`    | `mfxExtMoveRect.Rect[256].DestRight`                              | array         |
|                                                                 | `mfxU32`    | `mfxExtMoveRect.Rect[256].DestBottom`                             | array         |
|                                                                 | `mfxU32`    | `mfxExtMoveRect.Rect[256].SourceLeft`                             | array         |
|                                                                 | `mfxU32`    | `mfxExtMoveRect.Rect[256].SourceTop`                              | array         |
| mfxExtPartialBitstreamParam.BlockSize                           | `mfxU32`    | `mfxExtPartialBitstreamParam.BlockSize`                           |               |
| mfxExtPartialBitstreamParam.Granularity                         | `mfxU16`    | `mfxExtPartialBitstreamParam.Granularity`                         |               |
|                                                                 | `TimeStamp` | `mfxExtPictureTimingSEI[3].`                                      | array         |
| mfxExtPredWeightTable.LumaLog2WeightDenom                       | `mfxU16`    | `mfxExtPredWeightTable.LumaLog2WeightDenom`                       |               |
| mfxExtPredWeightTable.ChromaLog2WeightDenom                     | `mfxU16`    | `mfxExtPredWeightTable.ChromaLog2WeightDenom`                     |               |
|                                                                 | `mfxU16`    | `mfxExtPredWeightTable.LumaWeightFlag[2][32]`                     | nd-array      |
|                                                                 | `mfxU16`    | `mfxExtPredWeightTable.ChromaWeightFlag[2][32]`                   | nd-array      |
|                                                                 | `mfxI16`    | `mfxExtPredWeightTable.Weights[2][32][3][2]`                      | nd-array      |
| mfxExtTemporalLayers.NumLayers                                  | `mfxU16`    | `mfxExtTemporalLayers.NumLayers`                                  |               |
| mfxExtTemporalLayers.BaseLayerPID                               | `mfxU16`    | `mfxExtTemporalLayers.BaseLayerPID`                               |               |
|                                                                 | `mfxU16`    | `mfxExtTemporalLayers.Layers*.FrameRateScale`                     | pointer       |
|                                                                 | `mfxU32`    | `mfxExtTemporalLayers.Layers*.InitialDelayInKB`                   | pointer       |
|                                                                 | `mfxU32`    | `mfxExtTemporalLayers.Layers*.BufferSizeInKB`                     | pointer       |
|                                                                 | `mfxU32`    | `mfxExtTemporalLayers.Layers*.TargetKbps`                         | pointer       |
|                                                                 | `mfxU32`    | `mfxExtTemporalLayers.Layers*.MaxKbps`                            | pointer       |
|                                                                 | `mfxI32`    | `mfxExtTemporalLayers.Layers*.QPI`                                | pointer       |
|                                                                 | `mfxI32`    | `mfxExtTemporalLayers.Layers*.QPP`                                | pointer       |
|                                                                 | `mfxI32`    | `mfxExtTemporalLayers.Layers*.QPB`                                | pointer       |
| mfxExtThreadsParam.NumThread                                    | `mfxU16`    | `mfxExtThreadsParam.NumThread`                                    |               |
| mfxExtThreadsParam.SchedulingType                               | `mfxI32`    | `mfxExtThreadsParam.SchedulingType`                               |               |
| mfxExtThreadsParam.Priority                                     | `mfxI32`    | `mfxExtThreadsParam.Priority`                                     |               |
| mfxExtTimeCode.DropFrameFlag                                    | `mfxU16`    | `mfxExtTimeCode.DropFrameFlag`                                    |               |
| mfxExtTimeCode.TimeCodeHours                                    | `mfxU16`    | `mfxExtTimeCode.TimeCodeHours`                                    |               |
| mfxExtTimeCode.TimeCodeMinutes                                  | `mfxU16`    | `mfxExtTimeCode.TimeCodeMinutes`                                  |               |
| mfxExtTimeCode.TimeCodeSeconds                                  | `mfxU16`    | `mfxExtTimeCode.TimeCodeSeconds`                                  |               |
| mfxExtTimeCode.TimeCodePictures                                 | `mfxU16`    | `mfxExtTimeCode.TimeCodePictures`                                 |               |
| mfxExtVP9Param.FrameWidth                                       | `mfxU16`    | `mfxExtVP9Param.FrameWidth`                                       |               |
| mfxExtVP9Param.FrameHeight                                      | `mfxU16`    | `mfxExtVP9Param.FrameHeight`                                      |               |
| mfxExtVP9Param.WriteIVFHeaders                                  | `mfxU16`    | `mfxExtVP9Param.WriteIVFHeaders`                                  |               |
| mfxExtVP9Param.QIndexDeltaLumaDC                                | `mfxI16`    | `mfxExtVP9Param.QIndexDeltaLumaDC`                                |               |
| mfxExtVP9Param.QIndexDeltaChromaAC                              | `mfxI16`    | `mfxExtVP9Param.QIndexDeltaChromaAC`                              |               |
| mfxExtVP9Param.QIndexDeltaChromaDC                              | `mfxI16`    | `mfxExtVP9Param.QIndexDeltaChromaDC`                              |               |
| mfxExtVP9Param.NumTileRows                                      | `mfxU16`    | `mfxExtVP9Param.NumTileRows`                                      |               |
| mfxExtVP9Param.NumTileColumns                                   | `mfxU16`    | `mfxExtVP9Param.NumTileColumns`                                   |               |
| mfxExtVP9Segmentation.NumSegments                               | `mfxU16`    | `mfxExtVP9Segmentation.NumSegments`                               |               |
|                                                                 | `mfxU16`    | `mfxExtVP9Segmentation.Segment[8].FeatureEnabled`                 | array         |
|                                                                 | `mfxI16`    | `mfxExtVP9Segmentation.Segment[8].QIndexDelta`                    | array         |
|                                                                 | `mfxI16`    | `mfxExtVP9Segmentation.Segment[8].LoopFilterLevelDelta`           | array         |
|                                                                 | `mfxU16`    | `mfxExtVP9Segmentation.Segment[8].ReferenceFrame`                 | array         |
| mfxExtVP9Segmentation.SegmentIdBlockSize                        | `mfxU16`    | `mfxExtVP9Segmentation.SegmentIdBlockSize`                        |               |
| mfxExtVP9Segmentation.NumSegmentIdAlloc                         | `mfxU32`    | `mfxExtVP9Segmentation.NumSegmentIdAlloc`                         |               |
|                                                                 | `mfxU8`     | `mfxExtVP9Segmentation.SegmentId*`                                | pointer       |
|                                                                 | `mfxU16`    | `mfxExtVP9TemporalLayers.Layer[8].FrameRateScale`                 | array         |
|                                                                 | `mfxU16`    | `mfxExtVP9TemporalLayers.Layer[8].TargetKbps`                     | array         |
| mfxExtVPP3DLut.ChannelMapping                                   | `int`       | `mfxExtVPP3DLut.ChannelMapping`                                   |               |
| mfxExtVPP3DLut.BufferType                                       | `int`       | `mfxExtVPP3DLut.BufferType`                                       |               |
|                                                                 | `int`       | `mfxExtVPP3DLut.SystemBuffer.Channel[3].DataType`                 | array         |
|                                                                 | `mfxU32`    | `mfxExtVPP3DLut.SystemBuffer.Channel[3].Size`                     | array         |
|                                                                 | `mfxU8`     | `mfxExtVPP3DLut.SystemBuffer.Channel[3].Data*`                    | pointer,array |
|                                                                 | `mfxU16`    | `mfxExtVPP3DLut.SystemBuffer.Channel[3].Data16*`                  | pointer,array |
| mfxExtVPP3DLut.VideoBuffer.DataType                             | `int`       | `mfxExtVPP3DLut.VideoBuffer.DataType`                             |               |
| mfxExtVPP3DLut.VideoBuffer.MemLayout                            | `int`       | `mfxExtVPP3DLut.VideoBuffer.MemLayout`                            |               |
|                                                                 | `mfxMemId`  | `mfxExtVPP3DLut.VideoBuffer.MemId*`                               | pointer       |
| mfxExtVPPColorFill.Enable                                       | `mfxU16`    | `mfxExtVPPColorFill.Enable`                                       |               |
| mfxExtVPPComposite.Y                                            | `mfxU16`    | `mfxExtVPPComposite.Y`                                            |               |
| mfxExtVPPComposite.R                                            | `mfxU16`    | `mfxExtVPPComposite.R`                                            |               |
| mfxExtVPPComposite.U                                            | `mfxU16`    | `mfxExtVPPComposite.U`                                            |               |
| mfxExtVPPComposite.G                                            | `mfxU16`    | `mfxExtVPPComposite.G`                                            |               |
| mfxExtVPPComposite.V                                            | `mfxU16`    | `mfxExtVPPComposite.V`                                            |               |
| mfxExtVPPComposite.B                                            | `mfxU16`    | `mfxExtVPPComposite.B`                                            |               |
| mfxExtVPPComposite.NumTiles                                     | `mfxU16`    | `mfxExtVPPComposite.NumTiles`                                     |               |
| mfxExtVPPComposite.NumInputStream                               | `mfxU16`    | `mfxExtVPPComposite.NumInputStream`                               |               |
|                                                                 | `mfxU32`    | `mfxExtVPPComposite.InputStream*.DstX`                            | pointer       |
|                                                                 | `mfxU32`    | `mfxExtVPPComposite.InputStream*.DstY`                            | pointer       |
|                                                                 | `mfxU32`    | `mfxExtVPPComposite.InputStream*.DstW`                            | pointer       |
|                                                                 | `mfxU32`    | `mfxExtVPPComposite.InputStream*.DstH`                            | pointer       |
|                                                                 | `mfxU16`    | `mfxExtVPPComposite.InputStream*.LumaKeyEnable`                   | pointer       |
|                                                                 | `mfxU16`    | `mfxExtVPPComposite.InputStream*.LumaKeyMin`                      | pointer       |
|                                                                 | `mfxU16`    | `mfxExtVPPComposite.InputStream*.LumaKeyMax`                      | pointer       |
|                                                                 | `mfxU16`    | `mfxExtVPPComposite.InputStream*.GlobalAlphaEnable`               | pointer       |
|                                                                 | `mfxU16`    | `mfxExtVPPComposite.InputStream*.GlobalAlpha`                     | pointer       |
|                                                                 | `mfxU16`    | `mfxExtVPPComposite.InputStream*.PixelAlphaEnable`                | pointer       |
|                                                                 | `mfxU16`    | `mfxExtVPPComposite.InputStream*.TileId`                          | pointer       |
| mfxExtVPPDeinterlacing.Mode                                     | `mfxU16`    | `mfxExtVPPDeinterlacing.Mode`                                     |               |
| mfxExtVPPDeinterlacing.TelecinePattern                          | `mfxU16`    | `mfxExtVPPDeinterlacing.TelecinePattern`                          |               |
| mfxExtVPPDeinterlacing.TelecineLocation                         | `mfxU16`    | `mfxExtVPPDeinterlacing.TelecineLocation`                         |               |
| mfxExtVPPDenoise.DenoiseFactor                                  | `mfxU16`    | `mfxExtVPPDenoise.DenoiseFactor`                                  |               |
| mfxExtVPPDenoise2.Mode                                          | `int`       | `mfxExtVPPDenoise2.Mode`                                          |               |
| mfxExtVPPDenoise2.Strength                                      | `mfxU16`    | `mfxExtVPPDenoise2.Strength`                                      |               |
| mfxExtVPPDetail.DetailFactor                                    | `mfxU16`    | `mfxExtVPPDetail.DetailFactor`                                    |               |
| mfxExtVPPDoNotUse.NumAlg                                        | `mfxU32`    | `mfxExtVPPDoNotUse.NumAlg`                                        |               |
|                                                                 | `mfxU32`    | `mfxExtVPPDoNotUse.AlgList*`                                      | pointer       |
| mfxExtVPPDoUse.NumAlg                                           | `mfxU32`    | `mfxExtVPPDoUse.NumAlg`                                           |               |
|                                                                 | `mfxU32`    | `mfxExtVPPDoUse.AlgList*`                                         | pointer       |
| mfxExtVPPFieldProcessing.Mode                                   | `mfxU16`    | `mfxExtVPPFieldProcessing.Mode`                                   |               |
| mfxExtVPPFieldProcessing.InField                                | `mfxU16`    | `mfxExtVPPFieldProcessing.InField`                                |               |
| mfxExtVPPFieldProcessing.OutField                               | `mfxU16`    | `mfxExtVPPFieldProcessing.OutField`                               |               |
| mfxExtVPPFrameRateConversion.Algorithm                          | `mfxU16`    | `mfxExtVPPFrameRateConversion.Algorithm`                          |               |
| mfxExtVPPImageStab.Mode                                         | `mfxU16`    | `mfxExtVPPImageStab.Mode`                                         |               |
| mfxExtVPPMirroring.Type                                         | `mfxU16`    | `mfxExtVPPMirroring.Type`                                         |               |
| mfxExtVPPProcAmp.Brightness                                     | `mfxF64`    | `mfxExtVPPProcAmp.Brightness`                                     |               |
| mfxExtVPPProcAmp.Contrast                                       | `mfxF64`    | `mfxExtVPPProcAmp.Contrast`                                       |               |
| mfxExtVPPProcAmp.Saturation                                     | `mfxF64`    | `mfxExtVPPProcAmp.Saturation`                                     |               |
| mfxExtVPPRotation.Angle                                         | `mfxU16`    | `mfxExtVPPRotation.Angle`                                         |               |
| mfxExtVPPScaling.ScalingMode                                    | `mfxU16`    | `mfxExtVPPScaling.ScalingMode`                                    |               |
| mfxExtVPPScaling.InterpolationMethod                            | `mfxU16`    | `mfxExtVPPScaling.InterpolationMethod`                            |               |
| mfxExtVPPVideoSignalInfo.In.TransferMatrix                      | `mfxU16`    | `mfxExtVPPVideoSignalInfo.In.TransferMatrix`                      |               |
| mfxExtVPPVideoSignalInfo.In.NominalRange                        | `mfxU16`    | `mfxExtVPPVideoSignalInfo.In.NominalRange`                        |               |
| mfxExtVPPVideoSignalInfo.Out.TransferMatrix                     | `mfxU16`    | `mfxExtVPPVideoSignalInfo.Out.TransferMatrix`                     |               |
| mfxExtVPPVideoSignalInfo.Out.NominalRange                       | `mfxU16`    | `mfxExtVPPVideoSignalInfo.Out.NominalRange`                       |               |
| mfxExtVPPVideoSignalInfo.TransferMatrix                         | `mfxU16`    | `mfxExtVPPVideoSignalInfo.TransferMatrix`                         |               |
| mfxExtVPPVideoSignalInfo.NominalRange                           | `mfxU16`    | `mfxExtVPPVideoSignalInfo.NominalRange`                           |               |
| mfxExtVideoSignalInfo.VideoFormat                               | `mfxU16`    | `mfxExtVideoSignalInfo.VideoFormat`                               |               |
| mfxExtVideoSignalInfo.VideoFullRange                            | `mfxU16`    | `mfxExtVideoSignalInfo.VideoFullRange`                            |               |
| mfxExtVideoSignalInfo.ColourDescriptionPresent                  | `mfxU16`    | `mfxExtVideoSignalInfo.ColourDescriptionPresent`                  |               |
| mfxExtVideoSignalInfo.ColourPrimaries                           | `mfxU16`    | `mfxExtVideoSignalInfo.ColourPrimaries`                           |               |
| mfxExtVideoSignalInfo.TransferCharacteristics                   | `mfxU16`    | `mfxExtVideoSignalInfo.TransferCharacteristics`                   |               |
| mfxExtVideoSignalInfo.MatrixCoefficients                        | `mfxU16`    | `mfxExtVideoSignalInfo.MatrixCoefficients`                        |               |
| mfxExtVppAuxData.SpatialComplexity                              | `mfxU32`    | `mfxExtVppAuxData.SpatialComplexity`                              |               |
| mfxExtVppAuxData.TemporalComplexity                             | `mfxU32`    | `mfxExtVppAuxData.TemporalComplexity`                             |               |
| mfxExtVppAuxData.PicStruct                                      | `mfxU16`    | `mfxExtVppAuxData.PicStruct`                                      |               |
| mfxExtVppAuxData.SceneChangeRate                                | `mfxU16`    | `mfxExtVppAuxData.SceneChangeRate`                                |               |
| mfxExtVppAuxData.RepeatedFrame                                  | `mfxU16`    | `mfxExtVppAuxData.RepeatedFrame`                                  |               |
| mfxExtVppMctf.FilterStrength                                    | `mfxU16`    | `mfxExtVppMctf.FilterStrength`                                    |               |
<!-- [[[end]]] (checksum: 55642d9b7a3488ad1251bfccbf294470) -->