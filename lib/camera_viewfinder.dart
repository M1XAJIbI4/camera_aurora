// SPDX-FileCopyrightText: Copyright 2023 Open Mobile Platform LLC <community@omp.ru>
// SPDX-License-Identifier: BSD-3-Clause
import 'package:camera_aurora/camera_aurora_platform_interface.dart';
import 'package:flutter/material.dart';

import 'camera_data.dart';

class CameraViewfinder extends StatefulWidget {
  const CameraViewfinder({
    super.key,
    required this.width,
    required this.height,
  });

  final double width;
  final double height;

  @override
  State<CameraViewfinder> createState() => _CameraViewfinderState();
}

class _CameraViewfinderState extends State<CameraViewfinder> {
  CameraState _cameraState = CameraState.fromJson({});

  @override
  initState() {
    super.initState();
    CameraAuroraPlatform.instance.startCapture(widget.width, widget.height);
    CameraAuroraPlatform.instance.onChangeState().listen((event) {
      if (mounted) {
        setState(() {
          _cameraState = event;
        });
      }
    });
  }

  @override
  void dispose() {
    CameraAuroraPlatform.instance.stopCapture();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    if (_cameraState.hasError()) {
      return Center(
        child: Text(
          'Error: ${_cameraState.error}',
          style:
              const TextStyle(fontWeight: FontWeight.bold, color: Colors.white),
        ),
      );
    } else if (_cameraState.isNotEmpty()) {
      if (_cameraState.width == 0 || _cameraState.height == 0) {
        return const SizedBox.shrink();
      }

      int turn = 0;
      bool isFront = _cameraState.id.contains('front');

      switch (_cameraState.mountAngle) {
        case 0:
          turn += 0;
          break;
        case 90:
          turn += 1;
          break;
        case 180:
          turn += 2;
          break;
        default: // 270
          turn += 3;
      }

      switch (_cameraState.rotationDisplay) {
        case 0:
          turn -= 0;
          break;
        case 90:
          turn -= isFront ? -1 : 1;
          break;
        case 180:
          turn -= 2;
          break;
        default: // 270
          turn -= isFront ? -3 : 3;
      }

      double cw = turn % 2 == 0 ? _cameraState.height : _cameraState.width;
      double ch = turn % 2 == 0 ? _cameraState.width : _cameraState.height;

      double height = widget.height;
      double width = widget.height * cw / ch;

      if (width > widget.width) {
        width = widget.width;
        height = widget.width * ch / cw;
      }

      return SizedBox(
        width: width,
        height: height,
        child: RotatedBox(
          quarterTurns: turn,
          child: Texture(textureId: _cameraState.textureId),
        ),
      );
    }
    return const SizedBox.shrink();
  }
}
