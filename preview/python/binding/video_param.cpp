//==============================================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
//==============================================================================
#include "vpl/preview/video_param.hpp"
#include "vpl_python.hpp"
namespace vpl = oneapi::vpl;

class image_plane {
public:
    image_plane(void *base,
                py::ssize_t item_size,
                py::ssize_t cols,
                py::ssize_t rows,
                py::ssize_t sample_pitch,
                py::ssize_t row_pitch,
                std::string format,
                std::string desc)
            : base(base),
              item_size(item_size),
              rows(rows),
              cols(cols),
              row_pitch(row_pitch),
              sample_pitch(sample_pitch),
              format(format),
              desc(desc) {}

    py::buffer_info buffer_info() {
        return py::buffer_info(base,
                               item_size,
                               format,
                               2,
                               { cols, rows },
                               { sample_pitch, row_pitch });
    }

    std::string get_desc() {
        return desc;
    }

private:
    void *base;
    py::ssize_t item_size;
    py::ssize_t rows;
    py::ssize_t cols;
    py::ssize_t row_pitch;
    py::ssize_t sample_pitch;
    std::string format;
    std::string desc;
};

void init_video_param(const py::module &m) {
    py::class_<image_plane, std::shared_ptr<image_plane>>(m, "image_plane", py::buffer_protocol())
        .def_buffer(&image_plane::buffer_info)
        .def_property_readonly("desc", &image_plane::get_desc);

    py::class_<vpl::video_param, std::shared_ptr<vpl::video_param>>(m, "video_param")
        .def_property_readonly("Mfx", &vpl::video_param::getMfx)
        .def_property("AllocId", &vpl::video_param::get_AllocId, &vpl::video_param::set_AllocId)
        .def_property("AsyncDepth",
                      &vpl::video_param::get_AsyncDepth,
                      &vpl::video_param::set_AsyncDepth)
        .def_property("Protected",
                      &vpl::video_param::get_Protected,
                      &vpl::video_param::set_Protected)
        .def_property("IOPattern",
                      &vpl::video_param::get_IOPattern,
                      &vpl::video_param::set_IOPattern,
                      "i/o memory pattern value.")
        // .def("set_extension_buffers", &vpl::video_param::set_extension_buffers)
        .def("__str__", [](const vpl::video_param *self) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });

    py::class_<vpl::frame_info, std::shared_ptr<vpl::frame_info>>(m, "frame_info")
        .def(py::init<>())
        .def_property("BitDepthLuma",
                      &vpl::frame_info::get_BitDepthLuma,
                      &vpl::frame_info::set_BitDepthLuma)
        .def_property("BitDepthChroma",
                      &vpl::frame_info::get_BitDepthChroma,
                      &vpl::frame_info::set_BitDepthChroma)
        .def_property("Shift", &vpl::frame_info::get_Shift, &vpl::frame_info::set_Shift)
        .def_property("FrameId", &vpl::frame_info::get_FrameId, &vpl::frame_info::set_FrameId)
        .def_property("FourCC",
                      &vpl::frame_info::get_FourCC,
                      &vpl::frame_info::set_FourCC,
                      "color format fourCC value.")
        .def_property("frame_size",
                      &vpl::frame_info::get_frame_size,
                      &vpl::frame_info::set_frame_size,
                      "frame size.")
        .def_property("ROI", &vpl::frame_info::get_ROI, &vpl::frame_info::set_ROI, "ROI.")
        .def_property("BufferSize",
                      &vpl::frame_info::get_BufferSize,
                      &vpl::frame_info::set_BufferSize,
                      "buffer size")
        .def_property("frame_rate",
                      &vpl::frame_info::get_frame_rate,
                      &vpl::frame_info::set_frame_rate,
                      "frame rate value.")
        .def_property("aspect_ratio",
                      &vpl::frame_info::get_aspect_ratio,
                      &vpl::frame_info::set_aspect_ratio,
                      "aspect ratio.")
        .def_property("PicStruct",
                      &vpl::frame_info::get_PicStruct,
                      &vpl::frame_info::set_PicStruct,
                      "picture structure value.")
        .def_property("ChromaFormat",
                      &vpl::frame_info::get_ChromaFormat,
                      &vpl::frame_info::set_ChromaFormat,
                      "chroma format value.")
        .def("__str__", [](const vpl::frame_info *self) {
            std::stringstream strs;
            vpl::operator<<(strs, *self);
            return strs.str();
        });

    py::class_<vpl::frame_data, std::shared_ptr<vpl::frame_data>>(m, "frame_data")
        .def(py::init<>())
        .def_property("MemType", &vpl::frame_data::get_mem_type, &vpl::frame_data::set_mem_type)
        .def_property("Pitch",
                      &vpl::frame_data::get_pitch,
                      &vpl::frame_data::set_pitch,
                      "pitch value.")
        .def_property("TimeStamp", &vpl::frame_data::get_TimeStamp, &vpl::frame_data::set_TimeStamp)
        .def_property("FrameOrder",
                      &vpl::frame_data::get_FrameOrder,
                      &vpl::frame_data::set_FrameOrder)
        .def_property("Locked", &vpl::frame_data::get_Locked, &vpl::frame_data::set_Locked)
        .def_property("Corrupted", &vpl::frame_data::get_Corrupted, &vpl::frame_data::set_Corrupted)
        .def_property("DataFlag", &vpl::frame_data::get_DataFlag, &vpl::frame_data::set_DataFlag)
        .def(
            "get_planes",
            [](vpl::frame_data *self, vpl::frame_info &info) {
                auto size   = info.get_frame_size();
                auto pitch  = self->get_pitch();
                auto width  = size.first;
                auto height = size.second;
                switch (info.get_FourCC()) {
                    case vpl::color_format_fourcc::yuy2:
                        //  YUV 4:2:2   8       2   w2xh1
                        {
                            auto ptr = self->get_plane_ptrs_1();
                            return std::vector{ image_plane(
                                ptr,
                                1,
                                width * 2,
                                height,
                                1,
                                pitch,
                                py::format_descriptor<uint8_t>::format(),
                                "YUYV") };
                        }
                    case vpl::color_format_fourcc::uyvy:
                        //  YUV 4:2:2   8       2   w2xh1
                        {
                            auto ptr = self->get_plane_ptrs_1();
                            return std::vector{ image_plane(
                                ptr,
                                1,
                                width * 2,
                                height,
                                1,
                                pitch,
                                py::format_descriptor<uint8_t>::format(),
                                "UYVY") };
                        }
                    case vpl::color_format_fourcc::bgra:
                        //  RGB 4:4:4   8       4   w4xh1
                        {
                            auto ptr = self->get_plane_ptrs_1();
                            return std::vector{ image_plane(
                                ptr,
                                1,
                                width * 4,
                                height,
                                1,
                                pitch,
                                py::format_descriptor<uint8_t>::format(),
                                "BGRA") };
                        }
                    case vpl::color_format_fourcc::bgr4:
                        //  RGB 4:4:4   8       4   w4xh1
                        {
                            auto ptr = self->get_plane_ptrs_1();
                            return std::vector{ image_plane(
                                ptr,
                                1,
                                width * 4,
                                height,
                                1,
                                pitch,
                                py::format_descriptor<uint8_t>::format(),
                                "BGRA") };
                        }
                    case vpl::color_format_fourcc::ayuv:
                        //  YUV 4:4:4   8       4   w4xh1
                        {
                            auto ptr = self->get_plane_ptrs_1();
                            return std::vector{ image_plane(
                                ptr,
                                1,
                                width * 4,
                                height,
                                1,
                                pitch,
                                py::format_descriptor<uint8_t>::format(),
                                "AYUV") };
                        }
                    case vpl::color_format_fourcc::y210:
                        //  YUV 4:2:2   10      4   w4xh1
                        {
                            auto ptr = self->get_plane_ptrs_1();
                            return std::vector{ image_plane(
                                ptr,
                                2,
                                width * 2,
                                height,
                                1,
                                pitch,
                                py::format_descriptor<uint16_t>::format(),
                                "YUYV") };
                        }
                    case vpl::color_format_fourcc::y216:
                        //  YUV 4:2:2   16      4   w4xh1
                        {
                            auto ptr = self->get_plane_ptrs_1();
                            return std::vector{ image_plane(
                                ptr,
                                2,
                                width * 2,
                                height,
                                1,
                                pitch,
                                py::format_descriptor<uint16_t>::format(),
                                "YUYV") };
                        }
                    case vpl::color_format_fourcc::y410:
                        //  YUV 4:4:4   10      4   w4xh1
                        {
                            auto ptr = self->get_plane_ptrs_1();
                            return std::vector{ image_plane(
                                ptr,
                                1,
                                width * 4,
                                height,
                                1,
                                pitch,
                                py::format_descriptor<uint16_t>::format(),
                                "A:2 VYU:10") };
                        }
                    case vpl::color_format_fourcc::a2rgb10:
                        //  RGB 4:4:4   10:2    4   w4xh1
                        {
                            auto ptr = self->get_plane_ptrs_1();
                            return std::vector{ image_plane(
                                ptr,
                                1,
                                width * 4,
                                height,
                                1,
                                pitch,
                                py::format_descriptor<uint8_t>::format(),
                                "A:2 RGB:10") };
                        }
                    case vpl::color_format_fourcc::y416:
                        //  YUV 4:4:4   16      8   w8xh1
                        {
                            auto ptr = self->get_plane_ptrs_1();
                            return std::vector{ image_plane(
                                ptr,
                                2,
                                width * 4,
                                height,
                                1,
                                pitch,
                                py::format_descriptor<uint16_t>::format(),
                                "AVYU") };
                        }
                    case vpl::color_format_fourcc::nv12:
                        //  YUV 4:2:0   8       1:1 w1xh1:w1xh/2    Y   UV
                        {
                            auto ptr = self->get_plane_ptrs_2();
                            auto p1  = ptr.first;
                            auto p2  = ptr.second;
                            return std::vector{
                                image_plane(p1,
                                            1,
                                            width,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "Y"),
                                image_plane(p2,
                                            1,
                                            width,
                                            height / 2,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "UV")
                            };
                        }
                    case vpl::color_format_fourcc::p010:
                        //  YUV 4:2:0   10      2:2 w2xh1:w2xh/2    Y   UV
                        {
                            auto ptr = self->get_plane_ptrs_2();
                            auto p1  = ptr.first;
                            auto p2  = ptr.second;
                            return std::vector{
                                image_plane(p1,
                                            2,
                                            width,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint16_t>::format(),
                                            "Y"),
                                image_plane(p2,
                                            2,
                                            width,
                                            height / 2,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint16_t>::format(),
                                            "UV")
                            };
                        }
                    case vpl::color_format_fourcc::p016:
                        //  YUV 4:2:0   16      2:2 w2xh1:w2xh/2    Y   UV
                        {
                            auto ptr = self->get_plane_ptrs_2();
                            auto p1  = ptr.first;
                            auto p2  = ptr.second;
                            return std::vector{
                                image_plane(p1,
                                            2,
                                            width,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint16_t>::format(),
                                            "Y"),
                                image_plane(p2,
                                            2,
                                            width,
                                            height / 2,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint16_t>::format(),
                                            "UV")
                            };
                        }
                    case vpl::color_format_fourcc::nv16:
                        //  YUV 4:2:2   8       1:1 w1xh1:w1xh1     Y   UV
                        {
                            auto ptr = self->get_plane_ptrs_2();
                            auto p1  = ptr.first;
                            auto p2  = ptr.second;
                            return std::vector{
                                image_plane(p1,
                                            1,
                                            width,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "Y"),
                                image_plane(p2,
                                            1,
                                            width,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "UV")
                            };
                        }
                    case vpl::color_format_fourcc::p210:
                        //  YUV 4:2:2   10      2:2 w2xh1:w2xh1     Y   UV
                        {
                            auto ptr = self->get_plane_ptrs_2();
                            auto p1  = ptr.first;
                            auto p2  = ptr.second;
                            return std::vector{
                                image_plane(p1,
                                            2,
                                            width,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint16_t>::format(),
                                            "Y"),
                                image_plane(p2,
                                            2,
                                            width,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint16_t>::format(),
                                            "UV")
                            };
                        }
                    case vpl::color_format_fourcc::i420:
                        //  YUV 4:2:0   8       1:1:1   w1xh1:w1xh/2:w1xh/2     Y   U   V
                        {
                            auto ptr = self->get_plane_ptrs_3();
                            auto p1  = std::get<0>(ptr);
                            auto p2  = std::get<1>(ptr);
                            auto p3  = std::get<2>(ptr);
                            return std::vector{
                                image_plane(p1,
                                            1,
                                            width,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "Y"),
                                image_plane(p2,
                                            1,
                                            width / 2,
                                            height / 2,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "U"),
                                image_plane(p3,
                                            1,
                                            width / 2,
                                            height / 2,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "V")
                            };
                        }
                    case vpl::color_format_fourcc::yv12:
                        //  YUV 4:2:0   8       1:1:1   w1xh1:w1xh/2:w1xh/2     Y   V   U
                        {
                            auto ptr = self->get_plane_ptrs_3();
                            auto p1  = std::get<0>(ptr);
                            auto p2  = std::get<1>(ptr);
                            auto p3  = std::get<2>(ptr);
                            return std::vector{
                                image_plane(p1,
                                            1,
                                            width,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "Y"),
                                image_plane(p2,
                                            1,
                                            width / 2,
                                            height / 2,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "U"),
                                image_plane(p3,
                                            1,
                                            width / 2,
                                            height / 2,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "V")
                            };
                        }
                    case vpl::color_format_fourcc::i010:
                        //  YUV 4:2:0   10      2:2:2   w2xh1:w1xh/2:w1xh/2     Y   U   V
                        {
                            auto ptr = self->get_plane_ptrs_3();
                            auto p1  = std::get<0>(ptr);
                            auto p2  = std::get<1>(ptr);
                            auto p3  = std::get<2>(ptr);
                            return std::vector{
                                image_plane(p1,
                                            2,
                                            width / 2,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint16_t>::format(),
                                            "Y"),
                                image_plane(p2,
                                            2,
                                            width / 2,
                                            height / 2,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint16_t>::format(),
                                            "U"),
                                image_plane(p3,
                                            2,
                                            width,
                                            height / 2,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint16_t>::format(),
                                            "V")
                            };
                        }
                    case vpl::color_format_fourcc::i210:
                        //  YUV 4:2:2   10      2:2:2   w2xh:wxh:wxh     Y   U   V
                        {
                            auto ptr = self->get_plane_ptrs_3();
                            auto p1  = std::get<0>(ptr);
                            auto p2  = std::get<1>(ptr);
                            auto p3  = std::get<2>(ptr);
                            return std::vector{
                                image_plane(p1,
                                            2,
                                            width,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint16_t>::format(),
                                            "Y"),
                                image_plane(p2,
                                            2,
                                            width / 2,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint16_t>::format(),
                                            "U"),
                                image_plane(p3,
                                            2,
                                            width / 2,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint16_t>::format(),
                                            "V")
                            };
                        }
                    case vpl::color_format_fourcc::i422:
                        //  YUV 4:2:2   8      1:1:1   w1xh1:w1xh/2:w1xh/2     Y   U   V
                        {
                            auto ptr = self->get_plane_ptrs_3();
                            auto p1  = std::get<0>(ptr);
                            auto p2  = std::get<1>(ptr);
                            auto p3  = std::get<2>(ptr);
                            return std::vector{
                                image_plane(p1,
                                            1,
                                            width,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "Y"),
                                image_plane(p2,
                                            1,
                                            width / 2,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "U"),
                                image_plane(p3,
                                            1,
                                            width / 2,
                                            height,
                                            1,
                                            pitch,
                                            py::format_descriptor<uint8_t>::format(),
                                            "V")
                            };
                        }
                    case vpl::color_format_fourcc::rgb465:
                    case vpl::color_format_fourcc::rgbp:
                    case vpl::color_format_fourcc::rgb3:
                    case vpl::color_format_fourcc::p8:
                    case vpl::color_format_fourcc::p8_texture:
                    case vpl::color_format_fourcc::argb16:
                    case vpl::color_format_fourcc::abgr16:
                    case vpl::color_format_fourcc::r16:
                    case vpl::color_format_fourcc::ayuv_rgb4:
                    case vpl::color_format_fourcc::nv21:
                    case vpl::color_format_fourcc::bgrp:
                        throw std::range_error("Format not known");
                }
                throw std::range_error("Format not known");
            },
            "Get Planes");

    py::class_<vpl::codec_video_param, vpl::video_param, std::shared_ptr<vpl::codec_video_param>>(
        m,
        "codec_video_param")
        .def_property("LowPower",
                      &vpl::codec_video_param::get_LowPower,
                      &vpl::codec_video_param::set_LowPower)
        .def_property("BRCParamMultiplier",
                      &vpl::codec_video_param::get_BRCParamMultiplier,
                      &vpl::codec_video_param::set_BRCParamMultiplier)
        .def_property("CodecId",
                      &vpl::codec_video_param::get_CodecId,
                      &vpl::codec_video_param::set_CodecId,
                      "codec fourCC value.")
        .def_property("CodecProfile",
                      &vpl::codec_video_param::get_CodecProfile,
                      &vpl::codec_video_param::set_CodecProfile)
        .def_property("CodecLevel",
                      &vpl::codec_video_param::get_CodecLevel,
                      &vpl::codec_video_param::set_CodecLevel)
        .def_property("NumThread",
                      &vpl::codec_video_param::get_NumThread,
                      &vpl::codec_video_param::set_NumThread)
        .def_property("frame_info",
                      &vpl::codec_video_param::get_frame_info,
                      &vpl::codec_video_param::set_frame_info,
                      "frame info value.")
        .def("__str__", [](const vpl::codec_video_param *self) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });

    py::class_<vpl::encoder_video_param,
               vpl::codec_video_param,
               std::shared_ptr<vpl::encoder_video_param>>(m, "encoder_video_param")
        .def(py::init<>())
        .def_property("TargetUsage",
                      &vpl::encoder_video_param::get_TargetUsage,
                      &vpl::encoder_video_param::set_TargetUsage,
                      "TargetUsage value.")
        .def_property("GopPicSize",
                      &vpl::encoder_video_param::get_GopPicSize,
                      &vpl::encoder_video_param::set_GopPicSize)
        .def_property("GopRefDist",
                      &vpl::encoder_video_param::get_GopRefDist,
                      &vpl::encoder_video_param::set_GopRefDist)
        .def_property("GopOptFlag",
                      &vpl::encoder_video_param::get_GopOptFlag,
                      &vpl::encoder_video_param::set_GopOptFlag)
        .def_property("IdrInterval",
                      &vpl::encoder_video_param::get_IdrInterval,
                      &vpl::encoder_video_param::set_IdrInterval)
        .def_property("RateControlMethod",
                      &vpl::encoder_video_param::get_RateControlMethod,
                      &vpl::encoder_video_param::set_RateControlMethod,
                      "rate control method value.")
        .def_property("InitialDelayInKB",
                      &vpl::encoder_video_param::get_InitialDelayInKB,
                      &vpl::encoder_video_param::set_InitialDelayInKB)
        .def_property("QPI", &vpl::encoder_video_param::get_QPI, &vpl::encoder_video_param::set_QPI)
        .def_property("Accuracy",
                      &vpl::encoder_video_param::get_Accuracy,
                      &vpl::encoder_video_param::set_Accuracy)
        .def_property("BufferSizeInKB",
                      &vpl::encoder_video_param::get_BufferSizeInKB,
                      &vpl::encoder_video_param::set_BufferSizeInKB)
        .def_property("TargetKbps",
                      &vpl::encoder_video_param::get_TargetKbps,
                      &vpl::encoder_video_param::set_TargetKbps)
        .def_property("QPP", &vpl::encoder_video_param::get_QPP, &vpl::encoder_video_param::set_QPP)
        .def_property("ICQQuality",
                      &vpl::encoder_video_param::get_ICQQuality,
                      &vpl::encoder_video_param::set_ICQQuality)
        .def_property("MaxKbps",
                      &vpl::encoder_video_param::get_MaxKbps,
                      &vpl::encoder_video_param::set_MaxKbps)
        .def_property("QPB", &vpl::encoder_video_param::get_QPB, &vpl::encoder_video_param::set_QPB)
        .def_property("Convergence",
                      &vpl::encoder_video_param::get_Convergence,
                      &vpl::encoder_video_param::set_Convergence)
        .def_property("NumSlice",
                      &vpl::encoder_video_param::get_NumSlice,
                      &vpl::encoder_video_param::set_NumSlice)
        .def_property("NumRefFrame",
                      &vpl::encoder_video_param::get_NumRefFrame,
                      &vpl::encoder_video_param::set_NumRefFrame)
        .def_property("EncodedOrder",
                      &vpl::encoder_video_param::get_EncodedOrder,
                      &vpl::encoder_video_param::set_EncodedOrder)

        .def("__str__", [](const vpl::encoder_video_param *self) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });

    py::class_<vpl::decoder_video_param,
               vpl::codec_video_param,
               std::shared_ptr<vpl::decoder_video_param>>(m, "decoder_video_param")
        .def(py::init<>())
        .def_property("DecodedOrder",
                      &vpl::decoder_video_param::get_DecodedOrder,
                      &vpl::decoder_video_param::set_DecodedOrder)
        .def_property("ExtendedPicStruct",
                      &vpl::decoder_video_param::get_ExtendedPicStruct,
                      &vpl::decoder_video_param::set_ExtendedPicStruct)
        .def_property("TimeStampCalc",
                      &vpl::decoder_video_param::get_TimeStampCalc,
                      &vpl::decoder_video_param::set_TimeStampCalc)
        .def_property("SliceGroupsPresent",
                      &vpl::decoder_video_param::get_SliceGroupsPresent,
                      &vpl::decoder_video_param::set_SliceGroupsPresent)
        .def_property("MaxDecFrameBuffering",
                      &vpl::decoder_video_param::get_MaxDecFrameBuffering,
                      &vpl::decoder_video_param::set_MaxDecFrameBuffering)
        .def_property("EnableReallocRequest",
                      &vpl::decoder_video_param::get_EnableReallocRequest,
                      &vpl::decoder_video_param::set_EnableReallocRequest)

        .def("__str__", [](const vpl::decoder_video_param *self) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });

    py::class_<vpl::jpeg_decoder_video_param,
               vpl::codec_video_param,
               std::shared_ptr<vpl::jpeg_decoder_video_param>>(m, "jpeg_decoder_video_param")
        .def(py::init<>())
        .def_property("JPEGChromaFormat",
                      &vpl::jpeg_decoder_video_param::get_JPEGChromaFormat,
                      &vpl::jpeg_decoder_video_param::set_JPEGChromaFormat)
        .def_property("Rotation",
                      &vpl::jpeg_decoder_video_param::get_Rotation,
                      &vpl::jpeg_decoder_video_param::set_Rotation)
        .def_property("JPEGColorFormat",
                      &vpl::jpeg_decoder_video_param::get_JPEGColorFormat,
                      &vpl::jpeg_decoder_video_param::set_JPEGColorFormat)
        .def_property("InterleavedDec",
                      &vpl::jpeg_decoder_video_param::get_InterleavedDec,
                      &vpl::jpeg_decoder_video_param::set_InterleavedDec)
        .def_property("SamplingFactorH",
                      &vpl::jpeg_decoder_video_param::get_SamplingFactorH,
                      &vpl::jpeg_decoder_video_param::set_SamplingFactorH)
        .def_property("SamplingFactorV",
                      &vpl::jpeg_decoder_video_param::get_SamplingFactorV,
                      &vpl::jpeg_decoder_video_param::set_SamplingFactorV)

        .def("__str__", [](const vpl::jpeg_decoder_video_param *self) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });

    py::class_<vpl::jpeg_encoder_video_param,
               vpl::codec_video_param,
               std::shared_ptr<vpl::jpeg_encoder_video_param>>(m, "jpeg_encoder_video_param")
        .def(py::init<>())
        .def_property("Interleaved",
                      &vpl::jpeg_encoder_video_param::get_Interleaved,
                      &vpl::jpeg_encoder_video_param::set_Interleaved)
        .def_property("Quality",
                      &vpl::jpeg_encoder_video_param::get_Quality,
                      &vpl::jpeg_encoder_video_param::set_Quality)
        .def_property("RestartInterval",
                      &vpl::jpeg_encoder_video_param::get_RestartInterval,
                      &vpl::jpeg_encoder_video_param::set_RestartInterval)

        .def("__str__", [](const vpl::jpeg_encoder_video_param *self) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });

    py::class_<vpl::vpp_video_param, vpl::video_param, std::shared_ptr<vpl::vpp_video_param>>(
        m,
        "vpp_video_param")
        .def(py::init<>())
        .def_property("in_frame_info",
                      &vpl::vpp_video_param::get_in_frame_info,
                      &vpl::vpp_video_param::set_in_frame_info,
                      "frame info in value.")
        .def_property("out_frame_info",
                      &vpl::vpp_video_param::get_out_frame_info,
                      &vpl::vpp_video_param::set_out_frame_info,
                      "frame info out value.")

        .def("__str__", [](const vpl::vpp_video_param *self) {
            std::stringstream strs;
            strs << *self;
            return strs.str();
        });
}