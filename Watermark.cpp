    #include "Watermark.h"
    #include "ui_Watermark.h"

    #include <QFileDialog>
    #include <QPushButton> 
    #include <qlabel.h>
    #include <qstring.h>
    #include <qpainter.h>
    #include <qtabwidget.h>
    #include <qslider.h>
    #include <qcolor.h>
    #include <qcolordialog.h>

#include <opencv2/opencv.hpp>

    Watermark::Watermark(QWidget* parent)
        : QMainWindow(parent)
        , ui(new Ui_WatermarkClass)
    {
        ui->setupUi(this);

        connect(ui->chooseButton, &QPushButton::clicked, this, &Watermark::onChooseButtonClicked);
        connect(ui->applyButton, &QPushButton::clicked, this, &Watermark::onApplyButtonClicked);
        connect(ui->saveButton, &QPushButton::clicked, this, &Watermark::onSaveButtonClicked);
        connect(ui->colorButton, &QPushButton::clicked, this, &Watermark::onColorButtonClicked);

        connect(ui->template1Button, &QPushButton::clicked, this, &Watermark::onTemplate1ButtonClicked);
        connect(ui->template2Button, &QPushButton::clicked, this, &Watermark::onTemplate2ButtonClicked);
    }

    Watermark::~Watermark() {
        delete ui;
    }

    void Watermark::resizeEvent(QResizeEvent* event) {
        QMainWindow::resizeEvent(event);

        if (!textImage.isNull()) {
            QPixmap pix(QPixmap::fromImage(image));
            ui->imageLabel->setPixmap(pix.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }   
    }

    void Watermark::onChooseButtonClicked() {
        image = QImage(QFileDialog::getOpenFileName());
        if (image.isNull()) {
            return;
        }
        editedImage = image.copy();
        textImage = image.copy();
        QPixmap pix(QPixmap::fromImage(image));
        ui->imageLabel->setPixmap(pix.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    }

    void Watermark::onApplyButtonClicked() {
        apply();

        return;
    }

    void Watermark::onSaveButtonClicked() {
        textImage.save("output.jpg", "jpg", 90);
    }

    void Watermark::onColorButtonClicked() {
        color = (QColorDialog::getColor());
    }

    void Watermark::onTemplate1ButtonClicked() {
        QImage background = image.copy();
        background.fill(Qt::white);

        editedImage.fill(Qt::white);
        addMargin(background, 0.03, 0.03, 0.5, 0.5);

        apply();
    }

    void Watermark::onTemplate2ButtonClicked() {
        QImage background = image.copy();
        
        cv::Mat mat(background.height(), background.width(), CV_8UC4, (void*)background.bits(), background.bytesPerLine());
        cv::Mat small;
        cv::resize(mat, small, cv::Size(), 0.10, 0.10);
        small.convertTo(small, -1, 0.9, 0);
        cv::GaussianBlur(small, small, cv::Size(0, 0), 15);
        cv::resize(small, mat, mat.size());
        background = QImage((const uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32).copy();

        addMargin(background, 0.1, 0.1, 0.5, 0.5);

        apply();
    }

    void Watermark::apply() {
        if (image.isNull() || editedImage.isNull()) {
            return;
        }

        textImage = editedImage.copy();

        QPainter painter(&textImage);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);

        QFont font("Arial", ui->textSizeSlider->value() * 5, QFont::Bold);
        painter.setFont(font);

        QColor transColor = QColor(color.red(), color.green(), color.blue(), ui->transparencySlider->value());
        painter.setPen(transColor);

        painter.drawText(textImage.width() * ui->xSlider->value() / 100.0,
                         textImage.height() * (1 - ui->ySlider->value() / 100.0),
                         ui->lineEdit->text());

        QPixmap pix(QPixmap::fromImage(textImage));
        ui->imageLabel->setPixmap(pix.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        return;
    }

    void Watermark::addMargin(QImage background, double xMarginRatio, double yMarginRatio, double centerXRatio, double centerYRatio) {
        if (image.isNull()) {
            return;
        }
        int base = std::max(image.width(), image.height());
        int xMargin = base * xMarginRatio;
        int yMargin = base * yMarginRatio;
        editedImage = background.scaled(image.width() + xMargin * 2, image.height() + yMargin * 2);

        QPainter painter(&editedImage);

        int x = centerXRatio * editedImage.width() - image.width() / 2;
        int y = centerYRatio * editedImage.height() - image.height() / 2;

        painter.drawImage(x, y, image);
    }
