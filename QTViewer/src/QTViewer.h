#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QTViewer.h"

class QTViewer : public QMainWindow
{
    Q_OBJECT

public:
    QTViewer(QWidget *parent = nullptr);
    ~QTViewer();

private:
    Ui::QTViewerClass ui;
};
