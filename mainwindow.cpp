#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QString>
#include <QtDebug>
#include <QStyle>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTime>

void paintWidget::init() {

    head = tail = &map[0][0];
    bonus = 0;
    dx = 1;
    dy = 0;
    snake.clear();
    border.clear();
    food.clear();
    foodCount = 0;
    gamestart = false;

    int snakelen = 2;
    int snakex = 5;
    int snakey = 5;
    for(int i = 0; i < xlen; i++) {
        for(int j = 0; j < ylen; j++) {
            map[i][j].x = i;
            map[i][j].y = j;
            if(i == 0 | j == 0 | i == (xlen-1) | j == (ylen-1)) {
                map[i][j].type = border_lable;
                border.append(&map[i][j]);
               }
            else
                map[i][j].type = empty_lable;
        }
    }
    for(int i = 0; i < snakelen; i++) {
        snake.append(&map[i+snakex][snakey]);
        map[i+snakex][snakey].type = snake_label;
    }
}

paintWidget::paintWidget(QMainWindow* parent)
    : QWidget(parent){
    this->setGeometry(10, 60, 600, 600);
    init();
    timer=new QTimer;
    QObject::connect(timer,SIGNAL(timeout()),this,SLOT(snakeMoveSlot()));
    this->setFocusPolicy(Qt::StrongFocus);
}

void paintWidget::paintEvent(QPaintEvent*)
{
    paint = new QPainter;
    paint->begin(this);
    paint->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    for(int i = 0; i < border.length(); i++)
        paint->drawRect(border[i]->x*xysize, border[i]->y*xysize, xysize, xysize);
    paint->setBrush(QBrush(Qt::green, Qt::SolidPattern));
    for(int i = 0; i < snake.length(); i++)
        paint->drawRect(snake[i]->x*xysize, snake[i]->y*xysize, xysize, xysize);
    paint->setBrush((QBrush(Qt::red, Qt::SolidPattern)));
    for(int i = 0; i < food.length(); i++)
        paint->drawEllipse(food[i]->x*xysize, food[i]->y*xysize, xysize, xysize);
    paint->end();
}

void paintWidget::getHeadTail() {
    head = snake.at(snake.length() - 1);
    tail = snake.at(0);
}

void paintWidget::mousePressEvent(QMouseEvent *event) {
    switch (event->button()) {
        case Qt::LeftButton:
            if(!gamestart) {
                QPoint mouse = event->pos();
                int temp_x = (mouse.x()) / 15;
                int temp_y = (mouse.y()) / 15;
                map[temp_x][temp_y].type = border_lable;
                border.append(&map[temp_x][temp_y]);
            }
            update();
    }
}

void paintWidget::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) {
        // 同方向或者反方向不做任何操作

        case Qt::Key_Left:
            if(dx == 0){
                dx = -1;
                dy = 0;
                moveSnake();
            }
            break;
        case Qt::Key_Right:
            if(dx == 0){
                dx = 1;
                dy = 0;
                moveSnake();
            }
            break;
        case Qt::Key_Up:
            if(dy == 0){
                dx = 0;
                dy = -1;
                moveSnake();
                }
            break;
        case Qt::Key_Down:
            if(dy == 0){
                dx = 0;
                dy = 1;
                moveSnake();
                }
            break;

        default:
            break;
        }
}

void paintWidget::drawSnake(int x, int y) {
    paint->begin(this);
    paint->setBrush(QBrush(Qt::green, Qt::SolidPattern));
    paint->drawRect(x*xysize, y*xysize, xysize, xysize);
    paint->end();
}

void paintWidget::drawFood(int x, int y) {
    paint->begin(this);
    paint->setBrush(QBrush(Qt::red, Qt::SolidPattern));
    paint->drawRect(x*xysize, y*xysize, xysize, xysize);
    paint->end();
}

void paintWidget::drawBorder(int x, int y) {
    paint->begin(this);
    paint->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    paint->drawRect(x*xysize, y*xysize, xysize, xysize);
    paint->end();
}

void paintWidget::createFood() {

    int foodx;
    int foody;
    srand((unsigned)time(0));
    do {
        foodx = rand()%xlen;
        foody = rand()%ylen;
    }while(((foodx == 0)||(foody == 0)||(foodx==xlen-1)||(foody==ylen-1)));
    map[foodx][foody].type = food_label;
    food.append(&map[foodx][foody]);
}

void paintWidget::moveSnake() {

    getHeadTail();
    Node temp = map[(head->x)+dx][(head->y)+dy];
    if(bonus == 0) {
        snake.removeFirst();
        tail->type = empty_lable;
    }
    else
        bonus--;
    if(temp.type == border_lable || temp.type == snake_label)
        gameOver();
    else {
        snake.append(&map[(head->x)+dx][(head->y)+dy]);
        if(temp.type == food_label) {
            food.removeFirst();
            bonus += 3;
            createFood();
        }
        else {
            map[(head->x)+dx][(head->y)+dy].type = snake_label;
        }
    }
    update();
}

void paintWidget::gameOver() {
    timer->stop();
    QMessageBox::information(this, "Message:", "Game is over! Do you want to try is again?", QMessageBox::Ok);
    init();
    update();
    emit restared();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Snake");

    mlabel = new QLabel(this);
    mlabel->setStyleSheet("font-size:20px;");
    mlabel->setGeometry(500, 30, 100, 20);
    mlabel->setText("移动路程:");
    mlabel->show();

    gamewidget = new paintWidget(this);
    ui->pauseButton->setEnabled(false);
    ui->continueButton->setEnabled(false);
    ui->saveButton->setEnabled(false);
    ui->restartButton->setEnabled(false);


    QObject::connect(ui->startButton, SIGNAL(clicked()), this->gamewidget, SLOT(startGameSlot()));
    QObject::connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startGameSlot()));
    QObject::connect(ui->continueButton, SIGNAL(clicked()), this->gamewidget, SLOT(continueGameSlot()));
    QObject::connect(ui->continueButton, SIGNAL(clicked()), this, SLOT(continueGameSlot()));
    QObject::connect(ui->pauseButton, SIGNAL(clicked()), this->gamewidget, SLOT(pauseGameSlot()));
    QObject::connect(ui->pauseButton, SIGNAL(clicked()), this, SLOT(pauseGameSlot()));
    QObject::connect(ui->restartButton, SIGNAL(clicked()), this->gamewidget, SLOT(restartGameSlot()));
    QObject::connect(ui->restartButton, SIGNAL(clicked()), this, SLOT(restartGameSlot()));
    QObject::connect(gamewidget, SIGNAL(restared()), this, SLOT(restartGameSlot()));
//    QObject::connect(this->gamewidget->timer, SIGNAL(timeout()), this->gamewidget, SLOT(snakeMoveSlot()));
//    QObject::connect(ui->actionstartGame, SIGNAL(tgered()), this, SLOT(startGameSlot()));
//    QObject::connect(ui->actionpauseGame, SIGNAL(triggered()), this, SLOT(startGameSlot()));
//    QObject::connect(ui->actioncontinueGame, SIGNAL(triggered()), this, SLOT(startGameSlot()));
//    QObject::connect(ui->actionrestartGame, SIGNAL(triggered()), this, SLOT(startGameSlot()));
//    QObject::connect(ui->actionsaveGame, SIGNAL(triggered()), this, SLOT(startGameSlot()));
//    QObject::connect(ui->actionloadGame, SIGNAL(triggered()), this, SLOT(startGameSlot()));
//    QObject::connect(ui->startButton, SIGNAL(clicked()), this->gamewidget, SLOT(startGameSlot()));
//    QObject::connect(ui->pauseButton, SIGNAL(clicked()), this, SLOT(pauseGameSlot()));
//    QObject::connect(ui->continueButton, SIGNAL(clicked()), this, SLOT(continueGameSlot()));
//    QObject::connect(ui->restartButton, SIGNAL(clicked()), this, SLOT(restartGameSlot()));
//    QObject::connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(saveGameSlot()));
//    QObject::connect(ui->loadButton, SIGNAL(clicked()), this, SLOT(loadGameSlot()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void paintWidget::startGameSlot() {
    createFood();
    timer->start(150);
}

void paintWidget::continueGameSlot() {
    timer->start(150);
}

void paintWidget::pauseGameSlot() {
    timer->stop();
}

void paintWidget::restartGameSlot() {
    timer->stop();
    init();
}

void MainWindow::startGameSlot() {
    ui->startButton->setEnabled(false);
    ui->pauseButton->setEnabled(true);
    ui->loadButton->setEnabled(false);
    QApplication::processEvents();
}

void MainWindow::pauseGameSlot()
{
    ui->restartButton->setEnabled(true);
    ui->pauseButton->setEnabled(false);
    ui->continueButton->setEnabled(true);
    ui->saveButton->setEnabled(true);
}

void MainWindow::restartGameSlot() {
    ui->pauseButton->setEnabled(false);
    ui->continueButton->setEnabled(false);
    ui->saveButton->setEnabled(false);
    ui->startButton->setEnabled(true);
    ui->restartButton->setEnabled(false);
    ui->loadButton->setEnabled(true);
}

void MainWindow::continueGameSlot() {
    ui->restartButton->setEnabled(false);
    ui->pauseButton->setEnabled(true);
    ui->continueButton->setEnabled(false);
    ui->saveButton->setEnabled(false);
}

void paintWidget::snakeMoveSlot() {
    moveSnake();
}




