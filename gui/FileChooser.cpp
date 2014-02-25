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
#include "FileChooser.h"
#include <QWidget>
#include "PreviewFileDialog.h"

FileChooser::FileChooser(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	connect(btnReset, SIGNAL(clicked(bool)), this, SLOT(onFileResetCalled()));
	connect(btnSelectFile, SIGNAL(clicked(bool)), this, SLOT(onFileChooseCalled()));
	fileFilter = "All files (*.*)";
}

void FileChooser::setFileName(QString newfileName)
{
	filename = newfileName;
	updateUI();
}

void FileChooser::onFileChooseCalled()
{
	PreviewFileDialog *dlg  = new PreviewFileDialog(this);
	dlg->setAcceptMode(QFileDialog::AcceptOpen);

	QStringList filters;
	filters << fileFilter;
	dlg->setNameFilters(filters);
	if (!dlg->exec())
		return;

	filename = dlg->selectedFiles().first();

	updateUI();
	emit contentsChanged();
}

void FileChooser::onFileResetCalled()
{
	filename = defaultFilename;
	updateUI();
	emit contentsChanged();
}

void FileChooser::updateUI()
{
	filenameTextBox->setText(filename);
}
