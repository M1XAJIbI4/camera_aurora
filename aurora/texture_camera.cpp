/**
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <camera_aurora/texture_camera.h>
#include <camera_aurora/yuv.h>
#include <camera_aurora/yuv_i420.h>
#include <camera_aurora/yuv_nv12.h>

#include <flutter/platform-types.h>
#include <flutter/platform-methods.h>

#include "ZXing/ReadBarcode.h"

TextureCamera::TextureCamera(const TextureRegistrar &plugin,
                             const CameraErrorHandler &onError,
                             const ChangeQRHandler &onChangeQR)
    : m_plugin(plugin)
    , m_onError(onError)
    , m_onChangeQR(onChangeQR)
    , m_manager(StreamCameraManager())
    , m_camera(nullptr)
{}

void TextureCamera::GetImageBase64(const TakeImageBase64Handler &takeImageBase64)
{
    m_takeImageBase64 = takeImageBase64;
    m_isTakeImageBase64 = true;
}

std::vector<Encodable> TextureCamera::GetAvailableCameras()
{
    std::vector<Encodable> cameras;
    auto count = m_manager->getNumberOfCameras();

    for (int index = 0; index < count; index++) {
        Aurora::StreamCamera::CameraInfo info;
        if (m_manager->getCameraInfo(index, info)) {
            cameras.push_back(std::map<Encodable, Encodable>{
                {"id", info.id},
                {"name", info.name},
                {"provider", info.provider},
                {"mountAngle", info.mountAngle},
            });
        }
    }

    return cameras;
}

std::map<Encodable, Encodable> TextureCamera::GetState()
{
    if (m_camera) {
        return std::map<Encodable, Encodable>{
            {"id", m_info.id},
            {"textureId", m_textureId},
            {"width", m_captureWidth},
            {"height", m_captureHeight},
            {"mountAngle", m_info.mountAngle},
            {"rotationDisplay", static_cast<int>(PlatformMethods::GetOrientation())},
            {"error", m_error},
        };
    }

    return std::map<Encodable, Encodable>{{"error", m_error}};
}

bool TextureCamera::CreateCamera(std::string cameraName)
{
    if (m_camera) {
        return true;
    }

    if (m_manager->init()) {
        if (auto count = m_manager->getNumberOfCameras()) {
            for (int index = 0; index < count; index++) {
                if (m_manager->getCameraInfo(index, m_info)) {
                    if (m_info.id == cameraName) {
                        m_camera = m_manager->openCamera(m_info.id);

                        std::vector<Aurora::StreamCamera::CameraCapability> caps;

                        if (m_camera && m_manager->queryCapabilities(m_info.id, caps)) {
                            m_cap = caps.back();
                            return true;
                        } else {
                            SendError("Stream camera error open camera");
                            return false;
                        }
                    }
                }
            }
        }
    }

    return false;
}

void TextureCamera::SendError(std::string error)
{
    m_error = error;
    m_onError();
}

std::map<Encodable, Encodable> TextureCamera::StartCapture(int width, int height)
{
    if (m_camera && !m_camera->captureStarted()) {
        m_viewWidth = width;
        m_viewHeight = height;

        ResizeFrame(width, height, m_info, m_cap, m_captureWidth, m_captureHeight);

        m_isStart = m_camera->startCapture(m_cap);

        if (!m_isStart) {
            Unregister();
            SendError("Stream camera error start capture");
        } else {
            m_camera->setListener(this);
        }
    }

    return GetState();
}

void TextureCamera::StopCapture()
{
    m_isStart = false;

    int index = 0;
    do {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(m_chromaStep == 1 ? 10 : 500 /* r7 */));
        index++;
    } while (m_isTakeImageBase64 && index < 200);

    if (m_camera && m_camera->captureStarted()) {
        m_camera->stopCapture();
        m_camera->setListener(nullptr);
    }
}

std::map<Encodable, Encodable> TextureCamera::Register(std::string cameraName)
{
    m_textureId = m_plugin.RegisterTexture(
        [this](size_t, size_t) -> std::optional<BufferVariant> {
            if (m_bits && m_captureWidth != 0 && m_captureHeight != 0) {
                return std::make_optional(BufferVariant(
                    FlutterPixelBuffer{m_bits, (size_t) m_captureWidth, (size_t) m_captureHeight}));
            }
            return std::nullopt;
        });

    if (CreateCamera(cameraName) && m_viewWidth != 0) {
        StartCapture(m_viewWidth, m_viewHeight);
    }

    return GetState();
}

std::map<Encodable, Encodable> TextureCamera::Unregister()
{
    m_bits = nullptr;

    if (m_camera) {
        m_isStart = false;
        m_camera->stopCapture();
        m_camera->setListener(nullptr);
        m_camera = nullptr;
    }

    m_plugin.UnregisterTexture(m_textureId);

    m_error = "";
    m_counter = 0;
    m_counter_qr = 0;
    m_textureId = 0;
    m_captureWidth = 0;
    m_captureHeight = 0;

    return GetState();
}

std::map<Encodable, Encodable> TextureCamera::ResizeFrame(int width, int height)
{
    if (m_isStart && !(width == m_captureWidth || height == m_captureHeight)) {
        ResizeFrame(width, height, m_info, m_cap, m_captureWidth, m_captureHeight);
    }
    return GetState();
}

void TextureCamera::ResizeFrame(int width,
                                int height,
                                Aurora::StreamCamera::CameraInfo info,
                                Aurora::StreamCamera::CameraCapability cap,
                                int &captureWidth,
                                int &captureHeight)
{
    auto cw = cap.width;
    auto ch = cap.height;

    auto dw = width < 500 ? 500 : width + 100;
    auto dh = height < 500 ? 500 : height + 100;

    if (info.mountAngle == 270 || info.mountAngle == 90) {
        cw = cap.height;
        ch = cap.width;
    }

    m_bits = nullptr;

    captureHeight = dh;
    captureWidth = (cw * dh) / ch;

    if (captureWidth > dw) {
        captureWidth = dw;
        captureHeight = (ch * dw) / cw;
    }
}

std::optional<std::shared_ptr<const Aurora::StreamCamera::YCbCrFrame>> TextureCamera::GetFrame(
    std::shared_ptr<Aurora::StreamCamera::GraphicBuffer> buffer)
{
    auto frame = buffer->mapYCbCr();

    if (m_isTakeImageBase64) {
        std::string base64 = "";

        if (frame->chromaStep == 1 /* I420 */) {
            base64 = yuv::YUVToBase64(frame->y,
                                      frame->cr,
                                      frame->cb,
                                      frame->width,
                                      frame->height,
                                      static_cast<int>(PlatformMethods::GetOrientation()),
                                      m_info.mountAngle,
                                      m_info.id.find("front") != std::string::npos ? -1 : 1);
        }

        else if (frame->chromaStep == 2 /* NV12 */) {
            base64 = yuv::YUVToBase64(frame->y,
                                      frame->cr,
                                      nullptr,
                                      frame->width,
                                      frame->height,
                                      static_cast<int>(PlatformMethods::GetOrientation()),
                                      m_info.mountAngle,
                                      m_info.id.find("front") != std::string::npos ? -1 : 1);
        }

        m_takeImageBase64(base64);
        m_isTakeImageBase64 = false;
        return std::nullopt;
    }

    m_counter += 1;

    if (m_counter < 0) {
        m_counter = 0;
    }

    if (m_counter % 3 == 0) {
        return std::nullopt;
    }

    return frame;
}

void TextureCamera::onCameraFrame(std::shared_ptr<Aurora::StreamCamera::GraphicBuffer> buffer)
{
    if (!m_isStart && !m_isTakeImageBase64) {
        return;
    }

    if (auto optional = GetFrame(buffer)) {
        auto frame = optional.value();

        m_chromaStep = frame->chromaStep;

        if (m_enableSearchQr) {
            SearchQr(frame);
        }

        if (frame->chromaStep == 1 /* I420 */) {
            auto result = yuv::I420Scale(frame->y,
                                         frame->cr,
                                         frame->cb,
                                         frame->width,
                                         frame->height,
                                         m_captureWidth,
                                         m_captureHeight);
            m_bits = yuv::I420ToARGB(result.y, result.u, result.v, result.width, result.height);
        } else if (frame->chromaStep == 2 /* NV12 */) {
            auto result = yuv::NV12Scale(frame->y,
                                         frame->cr,
                                         frame->width,
                                         frame->height,
                                         m_captureWidth,
                                         m_captureHeight);
            m_bits = yuv::NV12ToARGB(result.y, result.uv, result.width, result.height);
        }

        m_plugin.MarkTextureAvailable(m_textureId);
    }
}

void TextureCamera::onCameraError(const std::string &errorDescription)
{
    Unregister();
    SendError(errorDescription);
}

void TextureCamera::onCameraParameterChanged(
    [[maybe_unused]] Aurora::StreamCamera::CameraParameter parameter, const std::string &value)
{
    std::cout << "onCameraParameterChanged: " << value << std::endl;
}

void TextureCamera::EnableSearchQr(bool state)
{
    m_enableSearchQr = state;
    m_counter_qr = 0;
}

void TextureCamera::SearchQr(std::shared_ptr<const Aurora::StreamCamera::YCbCrFrame> frame)
{
    int size = frame->chromaStep == 1 ? 15 : 30;

    m_counter_qr += 1;

    if (m_counter_qr < 0) {
        m_counter_qr = 0;
    }

    if (m_counter_qr % size != 0) {
        return;
    }

    // 720p
    int width = 1280;
    int height = 720;

    std::shared_ptr<uint8_t> bits;

    if (frame->chromaStep == 1 /* I420 */) {
        auto result = yuv::I420Scale(frame->y,
                                     frame->cr,
                                     frame->cb,
                                     frame->width,
                                     frame->height,
                                     width,
                                     height);

        bits = yuv::I420ToARGB(result.y, result.u, result.v, result.width, result.height);
    } else if (frame->chromaStep == 2 /* NV12 */) {
        auto result
            = yuv::NV12Scale(frame->y, frame->cr, frame->width, frame->height, width, height);
        bits = yuv::NV12ToARGB(result.y, result.uv, result.width, result.height);
    }

    if (bits) {
        auto image = ZXing::ImageView(bits.get(), width, height, ZXing::ImageFormat::RGBX);

        #if PSDK_MAJOR == 5
            auto hints = ZXing::DecodeHints().setFormats(ZXing::BarcodeFormat::QRCode);
        #else
            auto hints = ZXing::DecodeHints().setFormats(ZXing::BarcodeFormat::QR_CODE);
        #endif
        
        auto result = ZXing::ReadBarcode(image, hints);
        if (result.isValid()) {
            auto ws = result.text();
            std::string text(ws.begin(), ws.end());
            m_onChangeQR(text);
        } else {
            m_onChangeQR("");
        }
    }
}
