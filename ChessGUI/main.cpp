
#include<QApplication>
#include<QPushButton>
#include<QString>
#include<QWidget>
#include<QVBoxLayout>

int main(int argc , char * argv[]){
    QApplication app(argc,argv);

    //create the main window
    QWidget window ;
    window.setWindowTitle("Chess");
    window.resize(500,500);

    //creating a verticle layout
    // QVBoxLayout *layout = new QVBoxLayout(&window);

    //creating button pointers
    QPushButton *startButton = new QPushButton("start" , &window);
    QPushButton *restartButton = new QPushButton("restart" , &window);
    QPushButton *exitButton = new QPushButton("exit" , &window);

    // startButton->resize(100,100);
    // restartButton->resize(100,100);
    // startButton->setFixedSize(50,50);
    // restartButton->setFixedSize(100,50);
    // exitButton->setFixedSize(50,50);

    //adding buttons to layout
      //layout size way 1 ;
    // layout->addWidget(startButton , 0 , Qt::AlignLeft);
    // layout->addWidget(restartButton , 0 , Qt::AlignLeft);
    // layout->addWidget(exitButton , 0 , Qt::AlignRight);
      //layout size way2 ;
      //using set geometry instead of setFixedSize
    //when we use setGeometry we dont use layout instead we make parent using &window
    startButton->setGeometry(50 , 50 , 200 , 50 );
    restartButton->setGeometry(100 , 100 , 200 , 50);
    exitButton->setGeometry(400 , 400 , 50 , 50);

    //we do this when we use layout->H,V,Grid
    // layout->addWidget(startButton );
    // layout->addWidget(restartButton );
    // layout->addWidget(exitButton );

    QObject::connect(
        startButton,
        &QPushButton::clicked,
        [startButton](){
            startButton->setText("Game Strated");
        });

    QObject::connect(
        restartButton,
        &QPushButton::clicked,
        [restartButton](){
            restartButton->setText("Game has been restarted");
        });

    QObject::connect(
        exitButton,
        &QPushButton::clicked,
        &app,
        &QApplication::quit
        );

    window.show();
    // startButton->show();
    // restartButton->show();

    return app.exec();
}