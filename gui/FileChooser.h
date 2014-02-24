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

#ifndef FILECHOOSER_H_
#define FILECHOOSER_H_

#include "ui_file_chooser.h"
#include <QWidget>

class FileChooser : public QWidget, Ui::FileChooser
{
	Q_OBJECT
public:
	FileChooser(QWidget *parent);

	/* Initialization*/
	void setDefaultFilename(QString filename) { defaultFilename = filename; }
	void setFileFilter(QString filter) { fileFilter = filter; }

	/* IO */
	QString getFileName() {return filename; }
	void setFileName(QString newfileName);

public slots:
	void onFileChooseCalled();
	void onFileResetCalled();
signals:
	void contentsChanged();

private:
	QString filename;
	QString defaultFilename;
	QString fileFilter;

	void updateUI();
};

#endif
