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
    #include <qpainterpath.h>

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
        connect(ui->template3Button, &QPushButton::clicked, this, &Watermark::onTemplate3ButtonClicked);
    }

    Watermark::~Watermark() {
        delete ui;
    }

    void Watermark::resizeEvent(QResizeEvent* event) {
        QMainWindow::resizeEvent(event);

        apply();
    }

    void Watermark::onChooseButtonClicked() {
        image = QImage(QFileDialog::getOpenFileName());
        if (image.isNull()) {
            return;
        }
        editedImage = image.copy();
        textImage = image.copy();

        apply();
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
        if (image.isNull()) {
            return;
        }

        editedImage = image.copy();
        QImage background = image.copy();
        background.fill(Qt::white);
        addBackground(&editedImage, &background, 0.03, 0.03, 0.5, 0.5);

        apply();
    }

    void Watermark::onTemplate2ButtonClicked() {
        if (image.isNull()) {
            return;
        }

        editedImage = image.copy();
        QImage background = image.copy();
        blur(&background);
        addBackground(&editedImage, &background, 0.1, 0.1, 0.1, 0.1);
        roundCorners(&editedImage, 0.02);

        apply();
    }

    void Watermark::onTemplate3ButtonClicked() {
        editedImage = image.copy();
        QImage background = image.copy();
        blur(&background);
        addBackground(&editedImage, &background, 0.05, 0.05, 0.05, 0.2);

        roundCorners(&editedImage, 0.03);

        int base = std::min(image.width(), image.height());
        double imageBottomR = 0.05 + static_cast<double>(image.height()) / base;

        addText(&editedImage, ui->ssLineEdit->text(), "Segoe UI", 0.04, 0.15, false, false,
            Qt::white, Qt::AlignLeft | Qt::AlignVCenter, 0.2, imageBottomR, 0.2, 0.0);

        addText(&editedImage, ui->fLineEdit->text(), "Segoe UI", 0.04, 0.15, false, false,
            Qt::white, Qt::AlignHCenter | Qt::AlignVCenter, 0.2, imageBottomR, 0.2, 0.0);

        addText(&editedImage, ui->isoLineEdit->text(), "Segoe UI", 0.04, 0.15, false, false,
            Qt::white, Qt::AlignRight | Qt::AlignVCenter, 0.2, imageBottomR, 0.2, 0.0);
        
        apply();
    }

    void Watermark::apply() {
        if (image.isNull() || editedImage.isNull() || textImage.isNull()) {
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

    void Watermark::addBackground(QImage *dest, const QImage *background, double topR, double leftR, double rightR, double bottomR) {
        if (dest->isNull() || background->isNull()) {
            return;
        }
        int base = std::min(image.width(), image.height());
        QImage temp = dest->copy();
        *dest = background->scaled(image.width() + leftR * base + rightR * base, image.height() + topR * base + bottomR * base);

        QPainter painter(dest);

        painter.drawImage(leftR * base, topR * base, temp);
    }

    void Watermark::blur(QImage *image) {
        if (image->isNull()) {
            return;
        }
        cv::Mat mat(image->height(), image->width(), CV_8UC4, (void*)image->bits(), image->bytesPerLine());
        cv::Mat small;
        cv::resize(mat, small, cv::Size(), 0.10, 0.10);
        small.convertTo(small, -1, 0.9, 0);
        cv::GaussianBlur(small, small, cv::Size(0, 0), 15);
        cv::resize(small, mat, mat.size());
        *image = QImage((const uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32).copy();
    }

    void Watermark::roundCorners(QImage *image, double radiusR) {
        if (image->isNull())
            return;

        QImage result(image->size(), QImage::Format_ARGB32);
        result.fill(Qt::transparent);

        QPainter painter(&result);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QPainterPath path;
        int radius = qMin(image->width(), image->height()) * radiusR;
        path.addRoundedRect(QRectF(0, 0, image->width(), image->height()), radius, radius);
        painter.setClipPath(path);
        painter.drawImage(0, 0, *image);
        *image = result;
    }

    void Watermark::addText(QImage *textImage, const QString text, QString font, double sizeR, double weightR, bool isItalic, bool isShadowed, QColor color, Qt::Alignment alignment,
        double leftR, double topR, double rightR, double bottomR) {


        QPainter painter(textImage);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setPen(color);

        int base = std::min(image.height(), image.width());

        if (isItalic)
            painter.setFont(QFont(font, sizeR * base, weightR * base, isItalic));
        else
            painter.setFont(QFont(font, sizeR * base, weightR * base));

        int left = base * leftR;
        int top = base * topR;
        int right = base * rightR;
        int bottom = base * bottomR;
        
        QRect rect(left, top, textImage->width() - left - right, textImage->height() - top - bottom);

        if (isShadowed) {
            QColor shadowColor = QColor(0, 0, 0, 160);
            QPoint offset(2, 2);
            painter.setPen(shadowColor);
            painter.drawText(rect.translated(offset), alignment, text);
        }

        //-------------------------------------------------------------------------------------
        //QPen oldPen = painter.pen();
        //QPen debugPen(QColor(255, 0, 0, 150));  // red, semi-transparent
        //debugPen.setWidth(6);  // Thicker line
        //painter.setPen(debugPen);
        //painter.drawRect(rect);
        //painter.setPen(oldPen);
        //-------------------------------------------------------------------------------------

        painter.setPen(color);
        painter.drawText(rect, alignment, text);
    }

    void Watermark::getLogo(QImage *logo) {
        if (ui->brandComboBox->currentIndex() == 1)
            *logo = QImage(":/Watermark/images/nikonLogoText.png");
        else if (ui->brandComboBox->currentIndex() == 2)
            *logo = QImage(":/Watermark/images/canonLogo.png");

    }

    void Watermark::addLogo(QImage* tar, const QImage* logo, double sizeR, double xR, double yR) {
        QPainter painter(tar);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        
        int width = sizeR * image.width();
        int height = sizeR * image.height();
        QImage scaledLogo = logo->scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        int x = xR * image.width() - scaledLogo.width() / 2;
        int y = yR * image.height() - scaledLogo.height() / 2;
        painter.drawImage(x, y, scaledLogo);
    }
