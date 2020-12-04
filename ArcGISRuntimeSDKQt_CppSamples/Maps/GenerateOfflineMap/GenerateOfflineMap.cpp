// [WriteFile Name=GenerateOfflineMap, Category=Maps]
// [Legal]
// Copyright 2017 Esri.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// [Legal]

#ifdef PCH_BUILD
#include "pch.hpp"
#endif // PCH_BUILD

#include "GenerateOfflineMap.h"

#include "Map.h"
#include "MapQuickView.h"
#include "AuthenticationManager.h"
#include "Portal.h"
#include "PortalItem.h"
#include "OfflineMapTask.h"
#include "GeometryEngine.h"
#include "Envelope.h"
#include "Point.h"

using namespace Esri::ArcGISRuntime;

const QString GenerateOfflineMap::s_webMapId = QStringLiteral("acc027394bc84c2fb04d1ed317aac674");

GenerateOfflineMap::GenerateOfflineMap(QQuickItem* parent /* = nullptr */):
  QQuickItem(parent)
{
}

void GenerateOfflineMap::init()
{
  // Register the map view for QML
  qmlRegisterType<MapQuickView>("Esri.Samples", 1, 0, "MapView");
  qmlRegisterType<GenerateOfflineMap>("Esri.Samples", 1, 0, "GenerateOfflineMapSample");
  qmlRegisterUncreatableType<AuthenticationManager>("Esri.Samples", 1, 0, "AuthenticationManager", "AuthenticationManager is uncreateable");
}

void GenerateOfflineMap::componentComplete()
{
  QQuickItem::componentComplete();

  // find QML MapView component
  m_mapView = findChild<MapQuickView*>("mapView");

  // Create a Portal Item for use by the Map and OfflineMapTask
  bool loginRequired = true;
  Portal* portal = new Portal(loginRequired, this);
  m_portalItem = new PortalItem(portal, webMapId(), this);

  // Create a map from the Portal Item
  m_map = new Map(m_portalItem, this);

  // Update property once map is done loading
  connect(m_map, &Map::doneLoading, this, [this](Error e)
  {
    if (!e.isEmpty())
      return;

    m_mapLoaded = true;
    emit mapLoadedChanged();
  });

  // Set map to map view
  m_mapView->setMap(m_map);

  // Create the OfflineMapTask with the PortalItem
  m_offlineMapTask = new OfflineMapTask(m_portalItem, this);

  // connect to the error signal
  connect(m_offlineMapTask, &OfflineMapTask::errorOccurred, this, [](Error e)
  {
    if (e.isEmpty())
      return;

    qDebug() << e.message() << e.additionalMessage();
  });
}

void GenerateOfflineMap::generateMapByExtent(double xCorner1, double yCorner1, double xCorner2, double yCorner2)
{
  // create an envelope from the QML rectangle corners
  const Point corner1 = m_mapView->screenToLocation(xCorner1, yCorner1);
  const Point corner2 = m_mapView->screenToLocation(xCorner2, yCorner2);
  const Envelope extent = Envelope(corner1, corner2);
  const Envelope mapExtent = GeometryEngine::project(extent, SpatialReference::webMercator());

  // connect to the signal for when the default parameters are generated
  connect(m_offlineMapTask, &OfflineMapTask::createDefaultGenerateOfflineMapParametersCompleted,
          this, [this](QUuid, const GenerateOfflineMapParameters& params)
  {
    // Take the map offline once the parameters are generated
    GenerateOfflineMapJob* generateJob = m_offlineMapTask->generateOfflineMap(params, m_tempPath.path() + "/offlinemap.mmpk");

    // check if there is a valid job
    if (generateJob)
    {
      // connect to the job's status changed signal
      connect(generateJob, &GenerateOfflineMapJob::jobStatusChanged, this, [this, generateJob]()
      {
        // connect to the job's status changed signal to know once it is done
        switch (generateJob->jobStatus()) {
        case JobStatus::Failed:
          emit updateStatus("Generate failed");
          emit hideWindow(5000, false);
          break;
        case JobStatus::NotStarted:
          emit updateStatus("Job not started");
          break;
        case JobStatus::Paused:
          emit updateStatus("Job paused");
          break;
        case JobStatus::Started:
          emit updateStatus("In progress");
          break;
        case JobStatus::Succeeded:
          // show any layer errors
          if (generateJob->result()->hasErrors())
          {
            QString layerErrors = "";
            const QMap<Layer*, Error>& layerErrorsMap = generateJob->result()->layerErrors();
            for (auto it = layerErrorsMap.cbegin(); it != layerErrorsMap.cend(); ++it)
            {
              layerErrors += it.key()->name() + ": " + it.value().message() + "\n";
            }
            emit showLayerErrors(layerErrors);
          }

          // show the map
          emit updateStatus("Complete");
          emit hideWindow(1500, true);
          m_mapView->setMap(generateJob->result()->offlineMap(this));
          break;
        default:
          break;
        }
      });

      // connect to progress changed signal
      connect(generateJob, &GenerateOfflineMapJob::progressChanged, this, [this, generateJob]()
      {
        emit updateProgress(generateJob->progress());
      });

      // connect to the error signal
      connect(generateJob, &GenerateOfflineMapJob::errorOccurred, this, [](Error e)
      {
        if (e.isEmpty())
          return;

        qDebug() << e.message() << e.additionalMessage();
      });

      // start the generate job
      generateJob->start();
    }
    else
    {
      emit updateStatus("Export failed");
      emit hideWindow(5000, false);
    }
  });

  // generate parameters
  m_offlineMapTask->createDefaultGenerateOfflineMapParameters(mapExtent);
}

AuthenticationManager* GenerateOfflineMap::authenticationManager() const
{
  return AuthenticationManager::instance();
}
