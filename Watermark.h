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
    QImage image;
    QImage editedImage; 
    QImage textImage;
    QImage background;
    QColor color;

    void onChooseButtonClicked();
    void onApplyButtonClicked();
    void onSaveButtonClicked();
    void onColorButtonClicked();

    void onTemplate1ButtonClicked();
    void onTemplate2ButtonClicked();

    void apply();
    void addMargin(QImage background, double xMarginRatio, double yMarginRatio, double centerXRatio, double centerYRatio);

private:
    Ui_WatermarkClass* ui;

protected:
    void resizeEvent(QResizeEvent* event) override;
};

#endif // WATERMARK_H
