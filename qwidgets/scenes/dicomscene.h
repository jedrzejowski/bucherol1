#pragma once

#include <QtCore>
#include <QtWidgets>

#include <gdcmImage.h>
#include <gdcmImageReader.h>
#include <gdcmStringFilter.h>
#include <qwidgets/dicomgraphics.h>

#include "sokar/pixel.h"
#include "sokar/exception.h"
#include "_classdef.h"
#include "scene.h"
#include "params.h"
#include "sceneavatar.h"
#include "indicators/pixel.h"
#include "indicators/imgorientation.h"

namespace Sokar {
	class DicomScene : public Sokar::Scene {
	Q_OBJECT

	protected:
		const gdcm::Image &gdcmImage;
		const gdcm::File &gdcmFile;
		const gdcm::DataSet &gdcmDataSet;
		gdcm::StringFilter gdcmStringFilter;

		std::vector<char> originBuffer;
		std::vector<Pixel> targetBuffer;
		uint dimX, dimY;
		quint64 area;

		QPixmap pixmap, iconPixmap;
		QGraphicsPixmapItem *pixmapItem = nullptr;

//region Indicators
	private:
		void initIndicators();

		PixelSpacingIndicator *pixelSpacingIndicator;
		ImageOrientationIndicator *imageOrientationIndicator;

		void initPixelSpacingIndicator();
		void initImageOrientationIndicator();
//endregion

	public:

		explicit DicomScene(SceneParams &sceneParams);

		~DicomScene() override;

		const gdcm::File &getGdcmFile() { return gdcmFile; }

		void reloadPixmap();

		const QPixmap *getPixmap() const {
			return &pixmap;
		}

		SceneAvatar *getAvatar();

		const QPixmap& getIcon();

	protected:
		virtual bool generatePixmap() = 0;

		DicomGraphics *parentGraphics() const {
			return (DicomGraphics *) this->parent();
		}


	public slots:

		void reposItems() override;

	};
}