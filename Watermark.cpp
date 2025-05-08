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

        QImage background = image.copy();
        background.fill(Qt::white);
        editedImage = addMargin(image, background, 0.03, 0.03, 0.5, 0.5).copy();

        apply();
    }

    void Watermark::onTemplate2ButtonClicked() {
        if (image.isNull()) {
            return;
        }

        QImage background = blurBackground(image).copy();

        editedImage = addMargin(image, background, 0.1, 0.1, 0.5, 0.5).copy();
        editedImage = roundCorners(editedImage, 0.02).copy();

        apply();
    }

    void Watermark::onTemplate3ButtonClicked() {
        if (image.isNull()) {
            return;
        }

        QImage background = blurBackground(image).copy();

        editedImage = addMargin(image, background, 0.07, 0.15, 0.5, 0.4).copy();
        editedImage = roundCorners(editedImage, 0.03).copy();

        editedImage = addText(editedImage, ui->ssLineEdit->text(), "Segoe UI", 0.03, 0.1, false, true, Qt::white, Qt::AlignLeft | Qt::AlignBottom, 0.3, 0.0, 0.3, 0.05).copy();
        editedImage = addText(editedImage, ui->fLineEdit->text(), "Segoe UI", 0.03, 0.1, false, true, Qt::white, Qt::AlignCenter | Qt::AlignBottom, 0.3, 0.0, 0.3, 0.05).copy();
        editedImage = addText(editedImage, ui->isoLineEdit->text(), "Segoe UI", 0.03, 0.1, false, true, Qt::white, Qt::AlignRight | Qt::AlignBottom, 0.3, 0.0, 0.3, 0.05).copy();

        QImage logo;
        if (ui->brandComboBox->currentIndex() == 1)
            logo = QImage(":/Watermark/images/nikonLogoText.png");
        
        editedImage = addLogo(editedImage, logo, 1.2, 0.5, 0.83);

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

    QImage Watermark::addMargin(const QImage image, const QImage background, 
        const double xMarginRatio, const double yMarginRatio, const double centerXRatio, const double centerYRatio) {

        QImage result;
        int base = std::max(image.width(), image.height());
        int xMargin = base * xMarginRatio;
        int yMargin = base * yMarginRatio;
        result = background.scaled(image.width() + xMargin * 2, image.height() + yMargin * 2);

        QPainter painter(&result);

        int x = centerXRatio * result.width() - image.width() / 2;
        int y = centerYRatio * result.height() - image.height() / 2;

        painter.drawImage(x, y, image);

        return result;
    }

    QImage Watermark::blurBackground(QImage image) {
        QImage background = image.copy();

        cv::Mat mat(background.height(), background.width(), CV_8UC4, (void*)background.bits(), background.bytesPerLine());
        cv::Mat small;
        cv::resize(mat, small, cv::Size(), 0.10, 0.10);
        small.convertTo(small, -1, 0.9, 0);
        cv::GaussianBlur(small, small, cv::Size(0, 0), 15);
        cv::resize(small, mat, mat.size());
        background = QImage((const uchar*)mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32).copy();

        return background;
    }

    QImage Watermark::roundCorners(QImage image, double radiusR) {
        if (image.isNull())
            return QImage();

        QImage result(image.size(), QImage::Format_ARGB32);
        result.fill(Qt::transparent);

        QPainter painter(&result);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QPainterPath path;
        int radius = qMin(image.width(), image.height()) * radiusR;
        path.addRoundedRect(QRectF(0, 0, image.width(), image.height()), radius, radius);
        painter.setClipPath(path);
        painter.drawImage(0, 0, image);

        return result;
    }

    QImage Watermark::addText(QImage image, const QString text, QString font, double sizeR,double weightR, bool isItalic, bool isShadowed, QColor color, Qt::Alignment alignment,
        double leftR, double topR, double rightR, double bottomR) {

        QImage result = image.copy();

        QPainter painter(&result);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setPen(color);

        if (isItalic)
            painter.setFont(QFont(font, sizeR * image.height(), weightR * image.height(), isItalic));
        else
            painter.setFont(QFont(font, sizeR * image.height(), weightR * image.height()));

        int left = image.width() * leftR;
        int top = image.height() * topR;
        int right = image.width() * rightR;
        int bottom = image.height() * bottomR;
        QRect rect = image.rect().adjusted(left, top, -right, -bottom);

        if (isShadowed) {
            QColor shadowColor = QColor(0, 0, 0, 160);
            QPoint offset(2, 2);
            painter.setPen(shadowColor);
            painter.drawText(rect.translated(offset), alignment, text);
        }

        painter.setPen(color);
        painter.drawText(rect, alignment, text);

        return result;
    }

    QImage Watermark::addLogo(const QImage image, const QImage logo, double sizeR, double xR, double yR) {
        QImage result = image.copy();
        QPainter painter(&result);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        
        int width = sizeR * logo.width();
        int height = sizeR * logo.height();
        QImage scaledLogo = logo.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        int x = xR * image.width() - scaledLogo.width() / 2;
        int y = yR * image.height() - scaledLogo.width() / 2;
        painter.drawImage(x, y, scaledLogo);

        return result;
    }
