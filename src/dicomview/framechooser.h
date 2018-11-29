#pragma once

#include <QtCore>
#include <QtWidgets>

#include "_classdef.h"

#include "scenes/dicomscene.h"
#include "scenes/dicomsceneset.h"

namespace Sokar {

	class FrameChooser : public QScrollArea {
	Q_OBJECT
	private:
		QWidget *content;
		QVBoxLayout *layout;
		QVector<SceneAvatar*> avatars;

	public:
		explicit FrameChooser(QWidget *parent = nullptr);

		void addSceneSet(DicomSceneSet *sceneSet);
		void addScene(DicomScene *scene);

	protected:
		void resizeEvent(QResizeEvent *event) override;

	private slots:
		void onAvatarClicked(SceneAvatar *avatar);

	signals:
		void resizeAvatars(int width);
		void selectSceneSignal(DicomScene *scene);
	};
}

