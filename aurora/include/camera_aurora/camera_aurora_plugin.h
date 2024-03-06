/*
 * SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_H
#define FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_H

#include <camera_aurora/globals.h>
#include <flutter/plugin-interface.h>

#include <camera_aurora/texture_camera.h>

#include <QImage>
#include <QtCore>

class PLUGIN_EXPORT CameraAuroraPlugin final : public QObject, public PluginInterface
{
    Q_OBJECT

public:
    CameraAuroraPlugin();
    void RegisterWithRegistrar(PluginRegistrar &registrar) override;

private:
    void RegisterMethods(PluginRegistrar &registrar);
    void RegisterEvents(PluginRegistrar &registrar);

    void onAvailableCameras(const MethodCall &call);
    void onCreateCamera(const MethodCall &call);
    void onResizeFrame(const MethodCall &call);
    void onDispose(const MethodCall &call);
    void onStartCapture(const MethodCall &call);
    void onStopCapture(const MethodCall &call);

    void onTakePicture(const MethodCall &call);

    void onStartVideoRecording(const MethodCall &call);
    void onStopVideoRecording(const MethodCall &call);
    void onPauseVideoRecording(const MethodCall &call);
    void onResumeVideoRecording(const MethodCall &call);

    void unimplemented(const MethodCall &call);

private:
    std::shared_ptr<TextureCamera> m_textureCamera;
    bool m_isEnableStateChanged = false;
};

#endif /* FLUTTER_PLUGIN_CAMERA_AURORA_PLUGIN_H */
