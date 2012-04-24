#include "TvShow.h"
#include "Globals.h"

#include <QApplication>
#include <QDebug>

TvShow::TvShow(QString dir, QObject *parent) :
    QObject(parent)
{
    m_dir = dir;
    m_infoLoaded = false;
    m_backdropImageChanged = false;
    m_posterImageChanged = false;
}

void TvShow::moveToMainThread()
{
    moveToThread(QApplication::instance()->thread());
    for (int i=0, n=m_episodes.count() ; i<n ; ++i)
        m_episodes[i]->moveToMainThread();
}

void TvShow::clear()
{
    m_showTitle = "";
    m_rating = 0;
    m_firstAired = QDate(2000, 02, 30); // invalid date
    m_certification = "";
    m_network = "";
    m_overview = "";
    m_genres.clear();
    m_actors.clear();
    m_backdrops.clear();
    m_posters.clear();
    m_seasonPosters.clear();
    m_backdropImageChanged = false;
    m_posterImageChanged = false;
    m_seasonPosterImages.clear();
    m_seasonPosterImagesChanged.clear();
}

void TvShow::addEpisode(TvShowEpisode *episode)
{
    m_episodes.append(episode);
}

int TvShow::episodeCount()
{
    return m_episodes.count();
}

bool TvShow::loadData(MediaCenterInterface *mediaCenterInterface)
{
    bool infoLoaded = mediaCenterInterface->loadTvShow(this);
    if (!infoLoaded) {
        QStringList dirParts = this->dir().split("/");
        if (dirParts.count() > 0)
            this->setName(dirParts.last());
    }
    m_infoLoaded = infoLoaded;
    return infoLoaded;
}

void TvShow::loadData(QString id, TvScraperInterface *tvScraperInterface)
{
    tvScraperInterface->loadTvShowData(id, this);
}

bool TvShow::saveData(MediaCenterInterface *mediaCenterInterface)
{
    bool saved = mediaCenterInterface->saveTvShow(this);
    if (!m_infoLoaded)
        m_infoLoaded = saved;

    foreach (TvShowEpisode *episode, episodes())
        mediaCenterInterface->saveTvShowEpisode(episode);

    return saved;
}

void TvShow::scraperLoadDone()
{
    emit sigLoaded();
}

void TvShow::loadImages(MediaCenterInterface *mediaCenterInterface)
{
    mediaCenterInterface->loadTvShowImages(this);
}

/*** GETTER ***/

QString TvShow::dir() const
{
    return m_dir;
}

QString TvShow::name() const
{
    return m_name;
}

QString TvShow::showTitle() const
{
    return m_showTitle;
}

qreal TvShow::rating() const
{
    return m_rating;
}

QDate TvShow::firstAired() const
{
    return m_firstAired;
}

QStringList TvShow::genres() const
{
    return m_genres;
}

QString TvShow::certification() const
{
    return m_certification;
}

QString TvShow::network() const
{
    return m_network;
}

QString TvShow::overview() const
{
    return m_overview;
}

QStringList TvShow::certifications() const
{
    QStringList certifications;
    foreach (TvShowEpisode *episode, m_episodes) {
        if (!certifications.contains(episode->certification()) && !episode->certification().isEmpty())
            certifications.append(episode->certification());
    }

    return certifications;
}

QList<Actor> TvShow::actors() const
{
    return m_actors;
}

QList<Actor*> TvShow::actorsPointer()
{
    QList<Actor*> actors;
    for (int i=0, n=m_actors.size() ; i<n ; i++)
        actors.append(&(m_actors[i]));
    return actors;
}

QList<Poster> TvShow::posters() const
{
    return m_posters;
}

QList<Poster> TvShow::backdrops() const
{
    return m_backdrops;
}

QImage *TvShow::posterImage()
{
    return &m_posterImage;
}

QImage *TvShow::backdropImage()
{
    return &m_backdropImage;
}

QImage *TvShow::seasonPosterImage(int season)
{
    if (!m_seasonPosterImages.contains(season))
        m_seasonPosterImages.insert(season, QImage());

    return &m_seasonPosterImages[season];
}

QList<Poster> TvShow::seasonPosters(int season) const
{
    if (!m_seasonPosters.contains(season))
        return QList<Poster>();

    return m_seasonPosters[season];
}

TvShowEpisode *TvShow::episode(int season, int episode)
{
    for (int i=0, n=m_episodes.count() ; i<n ; ++i) {
        if (m_episodes[i]->season() == season && m_episodes[i]->episode() == episode)
            return m_episodes[i];
    }
    return new TvShowEpisode(QStringList(), this);
}

bool TvShow::posterImageChanged() const
{
    return m_posterImageChanged;
}

bool TvShow::backdropImageChanged() const
{
    return m_backdropImageChanged;
}

bool TvShow::seasonPosterImageChanged(int season) const
{
    return m_seasonPosterImagesChanged.contains(season);
}

QList<int> TvShow::seasons()
{
    QList<int> seasons;
    foreach (TvShowEpisode *episode, m_episodes) {
        if (!seasons.contains(episode->season()) && episode->season() != -2)
            seasons.append(episode->season());
    }
    return seasons;
}

QList<TvShowEpisode*> TvShow::episodes()
{
    return m_episodes;
}

/*** SETTER ***/

void TvShow::setName(QString name)
{
    m_name = name;
}

void TvShow::setShowTitle(QString title)
{
    m_showTitle = title;
}

void TvShow::setRating(qreal rating)
{
    m_rating = rating;
}

void TvShow::setFirstAired(QDate aired)
{
    m_firstAired = aired;
}

void TvShow::setGenres(QStringList genres)
{
    m_genres = genres;
}

void TvShow::addGenre(QString genre)
{
    m_genres.append(genre);
}

void TvShow::setCertification(QString certification)
{
    m_certification = certification;
}

void TvShow::setNetwork(QString network)
{
    m_network = network;
}

void TvShow::setOverview(QString overview)
{
    m_overview = overview;
}

void TvShow::setActors(QList<Actor> actors)
{
    m_actors = actors;
}

void TvShow::addActor(Actor actor)
{
    m_actors.append(actor);
}

void TvShow::setPosters(QList<Poster> posters)
{
    m_posters = posters;
}

void TvShow::setPoster(int index, Poster poster)
{
    if (m_posters.size() < index)
        return;
    m_posters[index] = poster;
}

void TvShow::setBackdrops(QList<Poster> backdrops)
{
    m_backdrops.append(backdrops);
}

void TvShow::setBackdrop(int index, Poster backdrop)
{
    if (m_backdrops.size() < index)
        return;
    m_backdrops[index] = backdrop;
}

void TvShow::addPoster(Poster poster)
{
    m_posters.append(poster);
}

void TvShow::addBackdrop(Poster backdrop)
{
    m_backdrops.append(backdrop);
}

void TvShow::setPosterImage(QImage poster)
{
    m_posterImage = QImage(poster);
    m_posterImageChanged = true;
}

void TvShow::setBackdropImage(QImage backdrop)
{
    m_backdropImage = QImage(backdrop);
    m_backdropImageChanged = true;
}

void TvShow::addSeasonPoster(int season, Poster poster)
{
    if (!m_seasonPosters.contains(season)) {
        QList<Poster> posters;
        m_seasonPosters.insert(season, posters);
    }

    m_seasonPosters[season].append(poster);
}

void TvShow::setSeasonPosterImage(int season, QImage poster)
{
    if (m_seasonPosterImages.contains(season))
        m_seasonPosterImages[season] = poster;
    else
        m_seasonPosterImages.insert(season, poster);

    if (!m_seasonPosterImagesChanged.contains(season))
        m_seasonPosterImagesChanged.append(season);
}

/*** DEBUG ***/

QDebug operator<<(QDebug dbg, const TvShow &show)
{
    QString nl = "\n";
    QString out;
    out.append("TvShow").append(nl);
    out.append(QString("  Dir:           ").append(show.dir()).append(nl));
    out.append(QString("  Name:          ").append(show.name()).append(nl));
    out.append(QString("  ShowTitle:     ").append(show.showTitle()).append(nl));
    out.append(QString("  Rating:        %1").arg(show.rating()).append(nl));
    out.append(QString("  FirstAired:    ").append(show.firstAired().toString("yyyy-MM-dd")).append(nl));
    out.append(QString("  Certification: ").append(show.certification()).append(nl));
    out.append(QString("  Network:       ").append(show.network()).append(nl));
    out.append(QString("  Overview:      ").append(show.overview())).append(nl);
    foreach (const QString &genre, show.genres())
        out.append(QString("  Genre:         ").append(genre)).append(nl);
    foreach (const Actor &actor, show.actors()) {
        out.append(QString("  Actor:         ").append(nl));
        out.append(QString("    Name:  ").append(actor.name)).append(nl);
        out.append(QString("    Role:  ").append(actor.role)).append(nl);
        out.append(QString("    Thumb: ").append(actor.thumb)).append(nl);
    }
    /*
    foreach (const QString &studio, movie.studios())
        out.append(QString("  Studio:         ").append(studio)).append(nl);
    foreach (const QString &country, movie.countries())
        out.append(QString("  Country:       ").append(country)).append(nl);
    foreach (const Poster &poster, movie.posters()) {
        out.append(QString("  Poster:       ")).append(nl);
        out.append(QString("    ID:       ").append(poster.id)).append(nl);
        out.append(QString("    Original: ").append(poster.originalUrl.toString())).append(nl);
        out.append(QString("    Thumb:    ").append(poster.thumbUrl.toString())).append(nl);
    }
    foreach (const Poster &backdrop, movie.backdrops()) {
        out.append(QString("  Backdrop:       ")).append(nl);
        out.append(QString("    ID:       ").append(backdrop.id)).append(nl);
        out.append(QString("    Original: ").append(backdrop.originalUrl.toString())).append(nl);
        out.append(QString("    Thumb:    ").append(backdrop.thumbUrl.toString())).append(nl);
    }
    */
    dbg.nospace() << out;
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const TvShow *show)
{
    dbg.nospace() << *show;
    return dbg.space();
}
