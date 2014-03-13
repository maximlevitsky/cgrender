/*
	This file is part of CG4.

	Copyright (c) Inbar Donag and Maxim Levitsky

    CG4 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    CG4 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CG4.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "PreviewFileDialog.h"
#include <QLabel>
#include <QGridLayout>

PreviewFileDialog::PreviewFileDialog(
	QWidget* parent,
	const QString & caption,
	const QString & directory,
	const QString & filter
) :
    QFileDialog(parent, caption, directory, filter)
{
	setObjectName("PreviewFileDialog");
	QVBoxLayout* box = new QVBoxLayout();

	mpPreview = new QLabel(tr("Preview"), this);
	mpPreview->setAlignment(Qt::AlignTop);
	mpPreview->setMinimumHeight(200);
	mpPreview->setMinimumWidth(200);
	box->addWidget(mpPreview);
	box->addStretch();
	QGridLayout *layout = (QGridLayout*)this->layout();
	layout->addLayout(box, 1, 3, 3, 1);

	connect(this, SIGNAL(currentChanged(const QString&)), this, SLOT(OnCurrentChanged(const QString&)));
}

void PreviewFileDialog::OnCurrentChanged(const QString & path)
{
	QPixmap pixmap = QPixmap(path);
	if (pixmap.isNull()) {
		mpPreview->setText("not an image");
	} else {
		mpPreview->setPixmap(pixmap.scaled(mpPreview->width(), mpPreview->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
}
