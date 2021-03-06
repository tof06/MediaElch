#include "LoadingStreamDetails.h"
#include "ui_LoadingStreamDetails.h"

LoadingStreamDetails::LoadingStreamDetails(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadingStreamDetails)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QFont font = ui->currentFile->font();
#ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize()-1);
#else
    font.setPointSize(font.pointSize()-2);
#endif
    ui->currentFile->setFont(font);
}

LoadingStreamDetails::~LoadingStreamDetails()
{
    delete ui;
}

void LoadingStreamDetails::loadMovies(QList<Movie*> movies)
{
    ui->progressBar->setRange(0, movies.count());
    ui->progressBar->setValue(0);
    ui->currentFile->clear();
    adjustSize();
    show();
    foreach (Movie *movie, movies) {
        movie->blockSignals(true);
        movie->controller()->loadStreamDetailsFromFile();
        movie->setChanged(true);
        movie->blockSignals(false);
        ui->progressBar->setValue(ui->progressBar->value()+1);
        ui->currentFile->setText(movie->name());
        qApp->processEvents();
    }
    accept();
}

void LoadingStreamDetails::loadConcerts(QList<Concert*> concerts)
{
    ui->progressBar->setRange(0, concerts.count());
    ui->progressBar->setValue(0);
    ui->currentFile->clear();
    adjustSize();
    show();
    foreach (Concert *concert, concerts) {
        concert->controller()->loadStreamDetailsFromFile();
        concert->setChanged(true);
        ui->progressBar->setValue(ui->progressBar->value()+1);
        ui->currentFile->setText(concert->name());
        qApp->processEvents();
    }
    accept();
}

void LoadingStreamDetails::loadTvShowEpisodes(QList<TvShowEpisode *> episodes)
{
    ui->progressBar->setRange(0, episodes.count());
    ui->progressBar->setValue(0);
    ui->currentFile->clear();
    adjustSize();
    show();
    foreach (TvShowEpisode *episode, episodes) {
        episode->loadStreamDetailsFromFile();
        episode->setChanged(true);
        ui->progressBar->setValue(ui->progressBar->value()+1);
        ui->currentFile->setText(episode->name());
        qApp->processEvents();
    }
    accept();
}
