/*
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <camera_aurora/camera_aurora_plugin.h>
#include <flutter/method-channel.h>
#include <flutter/platform-types.h>
#include <flutter/platform-events.h>
#include <flutter/platform-methods.h>

#include <QBuffer>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QMediaRecorder>
#include <QtCore>

#include <unistd.h>

namespace CameraAuroraMethods {
constexpr auto PluginKey = "camera_aurora";

constexpr auto AvailableCameras = "availableCameras";
constexpr auto CreateCamera = "createCamera";
constexpr auto ResizeFrame = "resizeFrame";
constexpr auto Dispose = "dispose";
constexpr auto StartCapture = "startCapture";
constexpr auto StopCapture = "stopCapture";
constexpr auto TakePicture = "takePicture";
} // namespace CameraAuroraMethods

namespace CameraAuroraEvents {
constexpr auto StateChanged = "cameraAuroraStateChanged";
constexpr auto QrChanged = "cameraAuroraQrChanged";
} // namespace CameraAuroraEvents

CameraAuroraPlugin::CameraAuroraPlugin()
{
    PlatformEvents::SubscribeOrientationChanged(
        [this]([[maybe_unused]] DisplayOrientation orientation) {
            if (this->m_isEnableStateChanged) {
                auto state = this->m_textureCamera->GetState();
                EventChannel(CameraAuroraEvents::StateChanged, MethodCodecType::Standard)
                    .SendEvent(state);
            }
        });
}

void CameraAuroraPlugin::RegisterWithRegistrar(PluginRegistrar &registrar)
{
    m_textureCamera = std::make_shared<TextureCamera>(registrar.GetTextureRegistrar(),
        [this]() {
            auto state = this->m_textureCamera->GetState();
            EventChannel(CameraAuroraEvents::StateChanged, MethodCodecType::Standard)
                .SendEvent(state);
        },
        [this](std::string data) {
            EventChannel(CameraAuroraEvents::QrChanged, MethodCodecType::Standard).SendEvent(data);
        });

    RegisterMethods(registrar);
    RegisterEvents(registrar);
}

void CameraAuroraPlugin::RegisterMethods(PluginRegistrar &registrar)
{
    auto methods = [this](const MethodCall &call) {
        const auto &method = call.GetMethod();

        if (method == CameraAuroraMethods::ResizeFrame) {
            onResizeFrame(call);
            return;
        }
        if (method == CameraAuroraMethods::AvailableCameras) {
            onAvailableCameras(call);
            return;
        }
        if (method == CameraAuroraMethods::CreateCamera) {
            onCreateCamera(call);
            return;
        }
        if (method == CameraAuroraMethods::StartCapture) {
            onStartCapture(call);
            return;
        }
        if (method == CameraAuroraMethods::StopCapture) {
            onStopCapture(call);
            return;
        }
        if (method == CameraAuroraMethods::Dispose) {
            onDispose(call);
            return;
        }
        if (method == CameraAuroraMethods::TakePicture) {
            onTakePicture(call);
            return;
        }

        unimplemented(call);
    };

    registrar.RegisterMethodChannel(CameraAuroraMethods::PluginKey,
                                    MethodCodecType::Standard,
                                    methods);
}

void CameraAuroraPlugin::RegisterEvents(PluginRegistrar &registrar)
{
    registrar.RegisterEventChannel(
        CameraAuroraEvents::StateChanged,
        MethodCodecType::Standard,
        [this](const Encodable &) {
            this->m_isEnableStateChanged = true;
            return EventResponse();
        },
        [this](const Encodable &) {
            this->m_isEnableStateChanged = true;
            return EventResponse();
        });

    registrar.RegisterEventChannel(
        CameraAuroraEvents::QrChanged,
        MethodCodecType::Standard,
        [this](const Encodable &) {
            if (m_textureCamera) {
                m_textureCamera->EnableSearchQr(true);
            }
            return EventResponse();
        },
        [this](const Encodable &) {
            if (m_textureCamera) {
                m_textureCamera->EnableSearchQr(false);
            }
            return EventResponse();
        });
}

void CameraAuroraPlugin::onResizeFrame(const MethodCall &call)
{
    auto width = call.GetArgument<Encodable::Int>("width");
    auto height = call.GetArgument<Encodable::Int>("height");

    auto state = m_textureCamera->ResizeFrame(width, height);

    EventChannel(CameraAuroraEvents::StateChanged, MethodCodecType::Standard).SendEvent(state);

    unimplemented(call);
}

void CameraAuroraPlugin::onAvailableCameras(const MethodCall &call)
{
    call.SendSuccessResponse(m_textureCamera->GetAvailableCameras());
}

void CameraAuroraPlugin::onCreateCamera(const MethodCall &call)
{
    auto cameraName = call.GetArgument<Encodable::String>("cameraName");

    auto state = m_textureCamera->Register(cameraName);

    EventChannel(CameraAuroraEvents::StateChanged, MethodCodecType::Standard).SendEvent(state);

    call.SendSuccessResponse(state);
}

void CameraAuroraPlugin::onStartCapture(const MethodCall &call)
{
    auto width = call.GetArgument<Encodable::Int>("width");
    auto height = call.GetArgument<Encodable::Int>("height");

    auto state = m_textureCamera->StartCapture(width, height);

    EventChannel(CameraAuroraEvents::StateChanged, MethodCodecType::Standard).SendEvent(state);

    unimplemented(call);
}

void CameraAuroraPlugin::onStopCapture(const MethodCall &call)
{
    m_textureCamera->StopCapture();

    unimplemented(call);
}

void CameraAuroraPlugin::onDispose(const MethodCall &call)
{
    auto state = m_textureCamera->Unregister();

    EventChannel(CameraAuroraEvents::StateChanged, MethodCodecType::Standard).SendEvent(state);

    unimplemented(call);
}

void CameraAuroraPlugin::onTakePicture(const MethodCall &call)
{
    m_textureCamera->GetImageBase64(
        [call](std::string base64) { call.SendSuccessResponse(base64); });
}

void CameraAuroraPlugin::unimplemented(const MethodCall &call)
{
    call.SendSuccessResponse(nullptr);
}

#include "moc_camera_aurora_plugin.cpp"
