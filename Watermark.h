#ifndef WATERMARK_H
#define WATERMARK_H

#include <QMainWindow>

class Ui_WatermarkClass;

class Watermark : public QMainWindow {
    Q_OBJECT

public:
    Watermark(QWidget* parent = nullptr);
    ~Watermark();

private slots:
    void onChooseButtonClicked();
    void onApplyButtonClicked();
    void onSaveButtonClicked();
    void onColorButtonClicked();

    void onTemplate1ButtonClicked();
    void onTemplate2ButtonClicked();
    void onTemplate3ButtonClicked();


private:
    Ui_WatermarkClass* ui;

    QImage image;
    QImage editedImage;
    QImage textImage;
    QColor color;

    void apply();
    
    void addBackground(QImage* dest, const QImage* background, double topR, double leftR, double rightR, double bottomR);

    void blur(QImage *image);

    void roundCorners(QImage *image, double radiusR);

    void addText(QImage *textImage, const QString text, QString font, double sizeR, double weightR, bool isItalic, bool isShadowed, QColor color, Qt::Alignment alignment,
        double leftR, double topR, double rightR, double bottomR);

    void getLogo(QImage* logo);

    void addLogo(QImage *tar, const QImage *logo, double sizeR, double xR, double yR);


protected:
    void resizeEvent(QResizeEvent* event) override;
};

#endif // WATERMARK_H
