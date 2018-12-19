#include <gdcmDict.h>
#include <gdcmDicts.h>
#include <gdcmGlobal.h>
#include <gdcmAttribute.h>
#include <gdcmDataElement.h>
#include <gdcmValue.h>
#include <gdcmTag.h>
#include <gdcmImageApplyLookupTable.h>
#include <gdcmStringFilter.h>
#include <gdcmImageHelper.h>

#include "sokar/speedtest.h"
#include "sokar/gdcmSokar.h"

#include "../../dicomview.h"
#include "../../graphics.h"

#include "monochrome.h"
#include "windowing/windowintdynamic.h"
#include "windowing/windowintstatic.h"

using namespace Sokar::Monochrome;

Scene::Scene(SceneParams &sceneParams) : DicomScene(sceneParams) {

	readAttributes();

	reloadPixmap();
}

Scene::~Scene() {

	delete imgWindow;
}

void Scene::readAttributes() {

	bool ok;
	ushort us;
	WindowInt *imgWindowInt;

	{ // Tworzenie obiekty okienka

		auto bitsStored = gdcmImage.GetPixelFormat().GetBitsStored();
		auto bitsAllocated = gdcmImage.GetPixelFormat().GetBitsAllocated();

		if (bitsAllocated > 62)
			throw Sokar::ImageTypeNotSupportedException();

		bool isDynamic = imgDimX * imgDimY < ((1 << bitsStored) - 1), isSigned = false;

		// Tworzenie odpowiedzniego okienka
		switch (gdcmImage.GetPixelFormat()) {
			case gdcm::PixelFormat::UINT8:
			case gdcm::PixelFormat::UINT12:
			case gdcm::PixelFormat::UINT16:
			case gdcm::PixelFormat::UINT32:
			case gdcm::PixelFormat::UINT64:
				isSigned = false;
				break;

			case gdcm::PixelFormat::INT8:
			case gdcm::PixelFormat::INT12:
			case gdcm::PixelFormat::INT16:
			case gdcm::PixelFormat::INT32:
			case gdcm::PixelFormat::INT64:
				isSigned = true;
				break;

			case gdcm::PixelFormat::FLOAT16:
			case gdcm::PixelFormat::FLOAT32:
			case gdcm::PixelFormat::FLOAT64:
				throw Sokar::ImageTypeNotSupportedException("gdcm::PixelFormat::FLOATXX");

			case gdcm::PixelFormat::UNKNOWN:
			case gdcm::PixelFormat::SINGLEBIT:
			default:
				throw Sokar::ImageTypeNotSupportedException();
		}

		if (isDynamic) imgWindow = new WindowIntDynamic(dataConverter);
		else imgWindow = new WindowIntStatic(dataConverter);

		addIndicator(imgWindow);

		imgWindowInt = (WindowInt *) imgWindow;
		imgWindowInt->setSigned(isSigned);

		auto maxValue = quint64(1) << bitsStored;
		if (isSigned) maxValue /= 2;
		imgWindowInt->setMaxValue(maxValue);

		imgWindowInt->setWidth(imgWindowInt->getMaxValue());
		imgWindowInt->setCenter(imgWindowInt->getMaxValue() / 2);

		connect(imgWindow, &Window::forceRefreshSignal, this, &DicomScene::reloadPixmap);
	}

	//Palette
	{
		switch (gdcmImage.GetPhotometricInterpretation()) {
			case gdcm::PhotometricInterpretation::MONOCHROME1:
				imgWindow->setPalette(Palette::getMono1());
				break;

			case gdcm::PhotometricInterpretation::MONOCHROME2:
				imgWindow->setPalette(Palette::getMono2());
				break;

			default:
				throw WrongScopeException(__FILE__, __LINE__);
		}
	}

	//
	{
		static gdcm::Tag
				TagWindowCenter(0x0028, 0x1050),
				TagWindowWidth(0x0028, 0x1051),
				TagWindowCenterWidthExplanation(0x0028, 0x1055);

		if (gdcmDataSet.FindDataElement(TagWindowCenter) && gdcmDataSet.FindDataElement(TagWindowWidth)) {

			auto centers = dataConverter.toDecimalString(TagWindowCenter);
			auto widths = dataConverter.toDecimalString(TagWindowWidth);
			QStringList names;

			if (gdcmDataSet.FindDataElement(TagWindowCenterWidthExplanation))
				names = dataConverter.toStringList(TagWindowCenterWidthExplanation);

			if (centers.size() != widths.size())
				throw DicomTagParseError(TagWindowWidth, "Number of WindowCenter's and WindowWidth's not match");

			for (int i = 0; i < centers.size(); i++) {

				imgWindowInt->pushDefaultValues(
						Monochrome::TrueInt(centers[i]),
						Monochrome::TrueInt(widths[i]),
						names.isEmpty() ? "" : names[i]
				);
			}

			imgWindowInt->setCenter(Monochrome::TrueInt(centers[0]));
			imgWindowInt->setWidth(Monochrome::TrueInt(widths[0]));

			goto endOfWindowBorders;
		}

		{
			goto endOfWindowBorders;
			TrueInt max, min;

			switch (gdcmImage.GetPixelFormat()) {
				case gdcm::PixelFormat::INT8:
					findExtremes<qint8>(max, min);
					break;

				case gdcm::PixelFormat::UINT8:
					findExtremes<quint8>(max, min);
					break;

				case gdcm::PixelFormat::INT16:
					findExtremes<qint16>(max, min);
					break;

				case gdcm::PixelFormat::UINT16:
					findExtremes<quint16>(max, min);
					break;

				case gdcm::PixelFormat::INT32:
					findExtremes<qint32>(max, min);
					break;

				case gdcm::PixelFormat::UINT32:
					findExtremes<quint32>(max, min);
					break;

				case gdcm::PixelFormat::INT64:
					findExtremes<qint64>(max, min);
					break;

				case gdcm::PixelFormat::UINT64:
					findExtremes<quint64>(max, min);
					break;

				default:
					throw Sokar::ImageTypeNotSupportedException();
			}

			qDebug() << qint64(min) << quint64(max);
			TrueInt diff = max - min;
			diff = ~diff + 1;

			imgWindowInt->setCenter(min + diff / 2);
			imgWindowInt->setWidth(diff);

			goto endOfWindowBorders;
		}

		endOfWindowBorders:;
	}

	//

	const static gdcm::Tag TagPixelPaddingValue(0x0028, 0x0120);

	if (gdcmDataSet.FindDataElement(TagPixelPaddingValue)) {
		//TODO ten tag może mieć różne wartości
		auto background = dataConverter.toString(TagPixelPaddingValue).toInt(&ok);

		if (!ok) throw DicomTagParseError(TagPixelPaddingValue);

		imgWindowInt->setBackgroundLvl(background);
	}

	//

	const static gdcm::Tag TagRescaleIntercept(0x0028, 0x1052), TagRescaleSlope(0x0028, 0x1053);

	if (gdcmDataSet.FindDataElement(TagRescaleIntercept)) {
		gdcm::assertTagPresence(gdcmDataSet, TagRescaleSlope);

		auto rescale = gdcm::ImageHelper::GetRescaleInterceptSlopeValue(gdcmFile);

		if (rescale.size() == 2) {
			imgWindowInt->setRescaleIntercept(rescale[0]);
			imgWindowInt->setRescaleSlope(rescale[1]);
		}
	}

	{
		if (gdcmImage.GetNumberOfOverlays() > 0)
			qDebug() << "Obraz z Overlayem (sprawdzić o co kaman)";
	}
}

bool Scene::generatePixmap() {

//	SpeedTest okienkowanie("LUT");

	if (!imgWindow->genLUT()) return false;

//	okienkowanie.close();

//	SpeedTest generowanie("Image");

	switch (gdcmImage.GetPixelFormat()) {
		case gdcm::PixelFormat::INT8:
			genQPixmapOfType<qint8>();
			break;

		case gdcm::PixelFormat::UINT8:
			genQPixmapOfType<quint8>();
			break;

		case gdcm::PixelFormat::INT16:
			genQPixmapOfType<qint16>();
			break;

		case gdcm::PixelFormat::UINT16:
			genQPixmapOfType<quint16>();
			break;

		case gdcm::PixelFormat::INT32:
			genQPixmapOfType<qint32>();
			break;

		case gdcm::PixelFormat::UINT32:
			genQPixmapOfType<quint32>();
			break;

		case gdcm::PixelFormat::INT64:
			genQPixmapOfType<qint64>();
			break;
		case gdcm::PixelFormat::UINT64:
			genQPixmapOfType<quint64>();
			break;

		default:
			throw Sokar::ImageTypeNotSupportedException();
	}

//	generowanie.close();

//	SpeedTest qting("QPixmap");

	pixmap.convertFromImage(qImage);

//	qting.close();

	return true;
}

template<typename IntType>
void Scene::findExtremes(TrueInt &max, TrueInt &min) {

	auto origin = (IntType *) &originBuffer[0];

	max = min = *origin;

	for (quint64 i = 0; i < imgDimX * imgDimY; i++, origin++) {
		max = max < *origin ? *origin : max;
		min = min > *origin ? *origin : min;
	}
}


template<typename IntType>
void Scene::genQPixmapOfType() {

	switch (imgWindow->type()) {
		case Window::IntDynamic:
			genQPixmapOfTypeWidthWindow<IntType, WindowIntDynamic>();
			break;

		case Window::IntStatic:
			genQPixmapOfTypeWidthWindow<IntType, WindowIntStatic>();
			break;

		default:
			throw WrongScopeException(__FILE__, __LINE__);
	}
}

template<typename IntType, typename WinClass>
void Scene::genQPixmapOfTypeWidthWindow() {

	std::vector<std::thread> threads;

	quint64 max = imgDimX * imgDimY;
	quint64 step = max / QThread::idealThreadCount();

	for (int i = 1; i < QThread::idealThreadCount(); i++) {
		std::thread t(&Scene::genQPixmapOfTypeWidthWindowThread<IntType, WinClass>,
					  this,
					  i * step,
					  std::min((i + 1) * step, max));

		threads.push_back(std::move(t));
	}

	genQPixmapOfTypeWidthWindowThread<IntType, WinClass>(0, step);

	for (auto &t: threads) t.join();
}

template<typename IntType, typename WinClass>
void Scene::genQPixmapOfTypeWidthWindowThread(quint64 from, quint64 to) {

	auto buffer = &targetBuffer[from];
	auto origin = (IntType *) &originBuffer[0];
	auto windowPtr = (WinClass *) imgWindow;

	origin += from;

	for (quint64 i = from; i < to; i++, origin++) {
		*buffer++ = windowPtr->getLUT(*origin);
	}
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {

	if (event->buttons() & Qt::LeftButton) {

		switch (getDicomView()->getToolBar()->getState()) {

			case DicomToolBar::Windowing: {
				event->accept();

				int dx = event->lastScreenPos().x() - event->screenPos().x(),
						dy = event->lastScreenPos().y() - event->screenPos().y();

				imgWindow->mvHorizontal(dx);
				imgWindow->mvVertical(dy);

				reloadPixmap();
			}
				break;
		}
	}

	DicomScene::mouseMoveEvent(event);
}

void Scene::toolBarAdjust() {
	DicomScene::toolBarAdjust();
	auto *toolBar = getDicomView()->getToolBar();

	auto winAction = toolBar->getActionWindowing();
	winAction->setMenu(imgWindow->getMenu());
	winAction->setEnabled(not isMovieMode());
}

bool Scene::acceptMovieMode(Sokar::MovieMode *movieMode) {
	if (not DicomScene::acceptMovieMode(movieMode))
		return false;

	//TODO dodać jakieś warunki

	return true;
}
