/**
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef TEXTURE_CAMERA_BUFFER_H
#define TEXTURE_CAMERA_BUFFER_H

#include <flutter/plugin-interface.h>
#include <streamcamera/streamcamera.h>

#include <chrono>
#include <thread>

typedef std::function<void()> CameraErrorHandler;
typedef std::function<void(std::string)> TakeImageBase64Handler;
typedef std::function<void(std::string)> ChangeQRHandler;

class TextureCamera : public Aurora::StreamCamera::CameraListener
{
public:
    TextureCamera(const TextureRegistrar &plugin,
                  const CameraErrorHandler &onError,
                  const ChangeQRHandler &onChangeQR);

    void onCameraError(const std::string &errorDescription) override;
    void onCameraFrame(std::shared_ptr<Aurora::StreamCamera::GraphicBuffer> buffer) override;
    void onCameraParameterChanged(Aurora::StreamCamera::CameraParameter,
                                  const std::string &value) override;

    std::vector<Encodable> GetAvailableCameras();
    std::map<Encodable, Encodable> Register(std::string cameraName);
    std::map<Encodable, Encodable> Unregister();
    std::map<Encodable, Encodable> StartCapture(int width, int height);
    void StopCapture();
    std::map<Encodable, Encodable> GetState();
    void GetImageBase64(const TakeImageBase64Handler &takeImageBase64);
    std::map<Encodable, Encodable> ResizeFrame(int width, int height);
    void EnableSearchQr(bool state);

private:
    void SearchQr(std::shared_ptr<const Aurora::StreamCamera::YCbCrFrame> frame);
    bool CreateCamera(std::string cameraName);
    void SendError(std::string error);
    void ResizeFrame(int width,
                     int height,
                     Aurora::StreamCamera::CameraInfo info,
                     Aurora::StreamCamera::CameraCapability cap,
                     int &captureWidth,
                     int &captureHeight);
    std::optional<std::shared_ptr<const Aurora::StreamCamera::YCbCrFrame>> GetFrame(
        std::shared_ptr<Aurora::StreamCamera::GraphicBuffer> buffer);

private:
    TextureRegistrar m_plugin;

    TakeImageBase64Handler m_takeImageBase64;
    CameraErrorHandler m_onError;
    ChangeQRHandler m_onChangeQR;
    std::string m_error;

    Aurora::StreamCamera::CameraInfo m_info;
    Aurora::StreamCamera::CameraCapability m_cap;

    std::shared_ptr<Aurora::StreamCamera::CameraManager> m_manager;
    std::shared_ptr<Aurora::StreamCamera::Camera> m_camera;
    std::shared_ptr<const Aurora::StreamCamera::YCbCrFrame> m_frame;

    int64_t m_textureId = 0;
    int m_captureWidth = 0;
    int m_captureHeight = 0;
    int m_viewWidth = 0;
    int m_viewHeight = 0;

    std::shared_ptr<uint8_t> m_bits;
    int m_counter = 0;
    int m_counter_qr = 0;
    int m_chromaStep = 1;
    bool m_isStart = false;
    bool m_isTakeImageBase64 = false;
    bool m_enableSearchQr = false;
};

#endif /* TEXTURE_CAMERA_BUFFER_H */
