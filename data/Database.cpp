#include "Database.h"

#include <QDesktopServices>
#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "mediaCenterPlugins/XbmcXml.h"
#include "settings/Settings.h"

/**
 * @brief Database::Database
 * @param parent
 */
Database::Database(QObject *parent) :
    QObject(parent)
{
    QString dataLocation = Settings::instance()->databaseDir();
    QDir dir(dataLocation);
    if (!dir.exists())
        dir.mkpath(dataLocation);
    m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "mediaDb"));
    m_db->setDatabaseName(dataLocation + QDir::separator() + "MediaElch.sqlite");
    if (!m_db->open()) {
        qWarning() << "Could not open cache database";
    } else {
        QSqlQuery query(*m_db);

        int dbVersion = 14;
        bool dbIsUpToDate = false;

        query.prepare("SELECT * FROM sqlite_master WHERE name ='settings' and type='table';");
        query.exec();
        if (query.next()) {
            query.prepare("SELECT value FROM settings WHERE idSettings=1");
            query.exec();
            if (query.next() && query.value(0).toInt() == dbVersion)
                dbIsUpToDate = true;
        }

        if (!dbIsUpToDate) {
            query.prepare("DROP TABLE IF EXISTS movies;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS movieFiles;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS concerts;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS concertFiles;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS shows;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS showsSettings;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS episodes;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS showsEpisodes;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS episodeFiles;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS settings;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS importCache;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS labels;");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS settings( "
                          "\"idSettings\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"value\" text NOT NULL "
                          ");");
            query.exec();
            query.prepare("INSERT INTO settings(idSettings, value) VALUES(1, :dbVersion)");
            query.bindValue(":dbVersion", QString::number(dbVersion));
            query.exec();
        }

        query.prepare("CREATE TABLE IF NOT EXISTS movies ( "
                      "\"idMovie\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"content\" text NOT NULL, "
                      "\"lastModified\" integer NOT NULL, "
                      "\"inSeparateFolder\" integer NOT NULL, "
                      "\"hasPoster\" integer NOT NULL, "
                      "\"hasBackdrop\" integer NOT NULL, "
                      "\"hasLogo\" integer NOT NULL, "
                      "\"hasClearArt\" integer NOT NULL, "
                      "\"hasCdArt\" integer NOT NULL, "
                      "\"hasBanner\" integer NOT NULL, "
                      "\"hasThumb\" integer NOT NULL, "
                      "\"hasExtraFanarts\" integer NOT NULL, "
                      "\"discType\" integer NOT NULL, "
                      "\"path\" text NOT NULL);");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS movieFiles( "
                      "\"idFile\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"idMovie\" integer NOT NULL, "
                      "\"file\" text NOT NULL "
                      ");");
        query.exec();
        query.prepare("CREATE INDEX id_movie_idx ON movieFiles(idMovie);");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS concerts ( "
                      "\"idConcert\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"content\" text NOT NULL, "
                      "\"inSeparateFolder\" integer NOT NULL, "
                      "\"path\" text NOT NULL);");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS concertFiles( "
                      "\"idFile\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"idConcert\" integer NOT NULL, "
                      "\"file\" text NOT NULL "
                      ");");
        query.exec();
        query.prepare("CREATE INDEX id_concert_idx ON concertFiles(idConcert);");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS shows ( "
                      "\"idShow\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"dir\" text NOT NULL, "
                      "\"content\" text NOT NULL, "
                      "\"path\" text NOT NULL);");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS showsSettings ( "
                      "\"idShow\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"tvdbid\" text NOT NULL, "
                      "\"url\" text NOT NULL, "
                      "\"showMissingEpisodes\" integer NOT NULL, "
                      "\"hideSpecialsInMissingEpisodes\" integer NOT NULL, "
                      "\"dir\" text NOT NULL);");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS showsEpisodes ( "
                      "\"idEpisode\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"content\" text NOT NULL, "
                      "\"idShow\" integer NOT NULL, "
                      "\"seasonNumber\" integer NOT NULL, "
                      "\"episodeNumber\" integer NOT NULL, "
                      "\"tvdbid\" text NOT NULL, "
                      "\"updated\" integer NOT NULL);");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS episodes ( "
                      "\"idEpisode\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"content\" text NOT NULL, "
                      "\"idShow\" integer NOT NULL, "
                      "\"seasonNumber\" integer NOT NULL, "
                      "\"episodeNumber\" integer NOT NULL, "
                      "\"path\" text NOT NULL);");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS episodeFiles( "
                      "\"idFile\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"idEpisode\" integer NOT NULL, "
                      "\"file\" text NOT NULL "
                      ");");
        query.exec();
        query.prepare("CREATE INDEX id_episode_idx ON episodeFiles(idEpisode);");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS labels ( "
                      "\"idLabel\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"color\" integer NOT NULL, "
                      "\"fileName\" text NOT NULL);");
        query.exec();
        query.prepare("CREATE INDEX id_label_filename_idx ON tags(fileName);");
        query.exec();


        query.prepare("CREATE TABLE IF NOT EXISTS importCache ( "
                      "\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"filename\" text NOT NULL, "
                      "\"type\" text NOT NULL, "
                      "\"path\" text NOT NULL);");
        query.exec();

        query.prepare("PRAGMA synchronous=0;");
        query.exec();

        query.prepare("PRAGMA cache_size=20000;");
        query.exec();
    }
}

/**
 * @brief Database::~Database
 */
Database::~Database()
{
    if (m_db && m_db->isOpen()) {
        m_db->close();
        delete m_db;
        m_db = 0;
    }
}

/**
 * @brief Returns an object to the cache database
 * @return Cache database object
 */
QSqlDatabase Database::db()
{
    return *m_db;
}

void Database::transaction()
{
    db().transaction();
}

void Database::commit()
{
    db().commit();
}

void Database::clearMovies(QString path)
{
    QSqlQuery query(db());
    if (!path.isEmpty()) {
        query.prepare("DELETE FROM movieFiles WHERE idMovie IN (SELECT idMovie FROM movies WHERE path=:path)");
        query.bindValue(":path", path.toUtf8());
        query.exec();
        query.prepare("DELETE FROM movies WHERE path=:path");
        query.bindValue(":path", path.toUtf8());
        query.exec();
    } else {
        query.prepare("DELETE FROM movies");
        query.exec();
        query.prepare("DELETE FROM sqlite_sequence WHERE name='movies'");
        query.exec();
        query.prepare("DELETE FROM movieFiles");
        query.exec();
        query.prepare("DELETE FROM sqlite_sequence WHERE name='movieFiles'");
        query.exec();
    }
}

void Database::add(Movie *movie, QString path)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO movies(content, lastModified, inSeparateFolder, hasPoster, hasBackdrop, hasLogo, hasClearArt, hasCdArt, hasBanner, hasThumb, hasExtraFanarts, discType, path) "
                  "VALUES(:content, :lastModified, :inSeparateFolder, :hasPoster, :hasBackdrop, :hasLogo, :hasClearArt, :hasCdArt, :hasBanner, :hasThumb, :hasExtraFanarts, :discType, :path)");
    query.bindValue(":content", movie->nfoContent().isEmpty() ? "" : movie->nfoContent().toUtf8());
    query.bindValue(":lastModified", movie->fileLastModified().isNull() ? QDateTime::currentDateTime() : movie->fileLastModified());
    query.bindValue(":inSeparateFolder", (movie->inSeparateFolder() ? 1 : 0));
    query.bindValue(":hasPoster", movie->hasImage(ImageType::MoviePoster) ? 1 : 0);
    query.bindValue(":hasBackdrop", movie->hasImage(ImageType::MovieBackdrop) ? 1 : 0);
    query.bindValue(":hasLogo", movie->hasImage(ImageType::MovieLogo) ? 1 : 0);
    query.bindValue(":hasClearArt", movie->hasImage(ImageType::MovieClearArt) ? 1 : 0);
    query.bindValue(":hasCdArt", movie->hasImage(ImageType::MovieCdArt) ? 1 : 0);
    query.bindValue(":hasBanner", movie->hasImage(ImageType::MovieBanner) ? 1 : 0);
    query.bindValue(":hasThumb", movie->hasImage(ImageType::MovieThumb) ? 1 : 0);
    query.bindValue(":hasExtraFanarts", movie->hasExtraFanarts() ? 1 : 0);
    query.bindValue(":discType", movie->discType());
    query.bindValue(":path", path.toUtf8());
    query.exec();
    int insertId = query.lastInsertId().toInt();

    foreach (const QString &file, movie->files()) {
        query.prepare("INSERT INTO movieFiles(idMovie, file) VALUES(:idMovie, :file)");
        query.bindValue(":idMovie", insertId);
        query.bindValue(":file", file.toUtf8());
        query.exec();
    }

    setLabel(movie->files(), movie->label());

    movie->setDatabaseId(insertId);
}

void Database::update(Movie *movie)
{
    QSqlQuery query(db());
    query.prepare("UPDATE movies SET content=:content WHERE idMovie=:id");
    query.bindValue(":content", movie->nfoContent().isEmpty() ? "" : movie->nfoContent());
    query.bindValue(":idMovie", movie->databaseId());
    query.exec();
}

QList<Movie*> Database::movies(QString path)
{
    QSqlQuery query(db());
    query.prepare("SELECT M.idMovie, M.content, M.lastModified, M.inSeparateFolder, M.hasPoster, M.hasBackdrop, M.hasLogo, M.hasClearArt, "
                  "M.hasCdArt, M.hasBanner, M.hasThumb, M.hasExtraFanarts, M.discType, MF.file, L.color "
                  "FROM movies M "
                  "LEFT JOIN movieFiles MF ON MF.idMovie=M.idMovie "
                  "LEFT JOIN labels L ON MF.file=L.fileName "
                  "WHERE path=:path "
                  "ORDER BY M.idMovie, MF.file");
    query.bindValue(":path", path.toUtf8());
    query.exec();
    QMap<int, Movie*> movies;
    while (query.next()) {
        if (!movies.contains(query.value(query.record().indexOf("idMovie")).toInt())) {
            int label = query.value(query.record().indexOf("color")).toInt();
            Movie *movie = new Movie(QStringList(), Manager::instance()->movieFileSearcher());
            movie->setDatabaseId(query.value(query.record().indexOf("idMovie")).toInt());
            movie->setFileLastModified(query.value(query.record().indexOf("lastModified")).toDateTime());
            movie->setInSeparateFolder(query.value(query.record().indexOf("inSeparateFolder")).toInt() == 1);
            movie->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
            movie->setHasImage(ImageType::MoviePoster, query.value(query.record().indexOf("hasPoster")).toInt() == 1);
            movie->setHasImage(ImageType::MovieBackdrop, query.value(query.record().indexOf("hasBackdrop")).toInt() == 1);
            movie->setHasImage(ImageType::MovieLogo, query.value(query.record().indexOf("hasLogo")).toInt() == 1);
            movie->setHasImage(ImageType::MovieClearArt, query.value(query.record().indexOf("hasClearArt")).toInt() == 1);
            movie->setHasImage(ImageType::MovieCdArt, query.value(query.record().indexOf("hasCdArt")).toInt() == 1);
            movie->setHasImage(ImageType::MovieBanner, query.value(query.record().indexOf("hasBanner")).toInt() == 1);
            movie->setHasImage(ImageType::MovieThumb, query.value(query.record().indexOf("hasThumb")).toInt() == 1);
            movie->setHasExtraFanarts(query.value(query.record().indexOf("hasExtraFanarts")).toInt() == 1);
            movie->setDiscType(static_cast<DiscType>(query.value(query.record().indexOf("discType")).toInt()));
            movie->setLabel(label);
            movies.insert(query.value(query.record().indexOf("idMovie")).toInt(), movie);
        }

        QStringList files = movies.value(query.value(query.record().indexOf("idMovie")).toInt())->files();
        files << query.value(query.record().indexOf("file")).toByteArray();
        movies.value(query.value(query.record().indexOf("idMovie")).toInt())->setFiles(files);
    }

    return movies.values();
}

void Database::clearConcerts(QString path)
{
    QSqlQuery query(db());
    if (!path.isEmpty()) {
        query.prepare("DELETE FROM concertFiles WHERE idConcert IN (SELECT idConcert FROM concerts WHERE path=:path)");
        query.bindValue(":path", path.toUtf8());
        query.exec();
        query.prepare("DELETE FROM concerts WHERE path=:path");
        query.bindValue(":path", path.toUtf8());
        query.exec();
    } else {
        query.prepare("DELETE FROM concerts");
        query.exec();
        query.prepare("DELETE FROM sqlite_sequence WHERE name='concerts'");
        query.exec();
        query.prepare("DELETE FROM concertFiles");
        query.exec();
        query.prepare("DELETE FROM sqlite_sequence WHERE name='concertFiles'");
        query.exec();
    }
}

void Database::add(Concert *concert, QString path)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO concerts(content, inSeparateFolder, path) "
                  "VALUES(:content, :inSeparateFolder, :path)");
    query.bindValue(":content", concert->nfoContent().isEmpty() ? "" : concert->nfoContent().toUtf8());
    query.bindValue(":inSeparateFolder", (concert->inSeparateFolder() ? 1 : 0));
    query.bindValue(":path", path.toUtf8());
    query.exec();
    int insertId = query.lastInsertId().toInt();

    foreach (const QString &file, concert->files()) {
        query.prepare("INSERT INTO concertFiles(idConcert, file) VALUES(:idConcert, :file)");
        query.bindValue(":idConcert", insertId);
        query.bindValue(":file", file.toUtf8());
        query.exec();
    }
    concert->setDatabaseId(insertId);
}

void Database::update(Concert *concert)
{
    QSqlQuery query(db());
    query.prepare("UPDATE concerts SET content=:content WHERE idConcert=:id");
    query.bindValue(":content", concert->nfoContent().isEmpty() ? "" : concert->nfoContent());
    query.bindValue(":id", concert->databaseId());
    query.exec();
}

QList<Concert*> Database::concerts(QString path)
{
    QList<Concert*> concerts;
    QSqlQuery query(db());
    QSqlQuery queryFiles(db());
    query.prepare("SELECT idConcert, content, inSeparateFolder FROM concerts WHERE path=:path");
    query.bindValue(":path", path.toUtf8());
    query.exec();
    while (query.next()) {
        QStringList files;
        queryFiles.prepare("SELECT file FROM concertFiles WHERE idConcert=:idConcert");
        queryFiles.bindValue(":idConcert", query.value(query.record().indexOf("idConcert")).toInt());
        queryFiles.exec();
        while (queryFiles.next())
            files << QString::fromUtf8(queryFiles.value(queryFiles.record().indexOf("file")).toByteArray());

        Concert *concert = new Concert(files, Manager::instance()->concertFileSearcher());
        concert->setDatabaseId(query.value(query.record().indexOf("idConcert")).toInt());
        concert->setInSeparateFolder(query.value(query.record().indexOf("inSeparateFolder")).toInt() == 1);
        concert->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        concerts.append(concert);
    }
    return concerts;
}

void Database::add(TvShow *show, QString path)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO shows(dir, content, path) "
                  "VALUES(:dir, :content, :path)");
    query.bindValue(":dir", show->dir().toUtf8());
    query.bindValue(":content", show->nfoContent().isEmpty() ? "" : show->nfoContent().toUtf8());
    query.bindValue(":path", path.toUtf8());
    query.exec();
    show->setDatabaseId(query.lastInsertId().toInt());

    query.prepare("SELECT showMissingEpisodes, hideSpecialsInMissingEpisodes FROM showsSettings WHERE dir=:dir");
    query.bindValue(":dir", show->dir().toUtf8());
    query.exec();
    if (query.next()) {
        show->setShowMissingEpisodes(query.value(query.record().indexOf("showMissingEpisodes")).toInt() == 1);
        show->setHideSpecialsInMissingEpisodes(query.value(query.record().indexOf("hideSpecialsInMissingEpisodes")).toInt() == 1);
    } else {
        query.prepare("INSERT INTO showsSettings(showMissingEpisodes, hideSpecialsInMissingEpisodes, dir, tvdbid, url) VALUES(0, 0, :dir, :tvdbid, :url)");
        query.bindValue(":dir", show->dir().toUtf8());
        query.bindValue(":tvdbid", show->tvdbId().isEmpty() ? "" : show->tvdbId());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.exec();
        show->setShowMissingEpisodes(false);
        show->setHideSpecialsInMissingEpisodes(false);
    }
}

void Database::setShowMissingEpisodes(TvShow *show, bool showMissing)
{
    QSqlQuery query(db());

    query.prepare("SELECT showMissingEpisodes FROM showsSettings WHERE dir=:dir");
    query.bindValue(":dir", show->dir().toUtf8());
    query.exec();
    if (query.next()) {
        query.prepare("UPDATE showsSettings SET showMissingEpisodes=:show, url=:url, tvdbid=:tvdbid WHERE dir=:dir");
        query.bindValue(":show", showMissing ? 1 : 0);
        query.bindValue(":dir", show->dir().toUtf8());
        query.bindValue(":tvdbid", show->tvdbId().isEmpty() ? "" : show->tvdbId());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.exec();
    } else {
        query.prepare("INSERT INTO showsSettings(showMissingEpisodes, dir, tvdbid, url) VALUES(:show, :dir, :tvdbid, :url)");
        query.bindValue(":dir", show->dir().toUtf8());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.bindValue(":tvdbid", show->tvdbId().isEmpty() ? "" : show->tvdbId());
        query.bindValue(":show", showMissing ? 1 : 0);
        query.exec();
    }
}

void Database::setHideSpecialsInMissingEpisodes(TvShow *show, bool hideSpecials)
{
    QSqlQuery query(db());

    query.prepare("SELECT hideSpecialsInMissingEpisodes FROM showsSettings WHERE dir=:dir");
    query.bindValue(":dir", show->dir().toUtf8());
    query.exec();
    if (query.next()) {
        query.prepare("UPDATE showsSettings SET hideSpecialsInMissingEpisodes=:hide, url=:url, tvdbid=:tvdbid WHERE dir=:dir");
        query.bindValue(":show", hideSpecials ? 1 : 0);
        query.bindValue(":dir", show->dir().toUtf8());
        query.bindValue(":tvdbid", show->tvdbId().isEmpty() ? "" : show->tvdbId());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.exec();
    } else {
        query.prepare("INSERT INTO showsSettings(hideSpecialsInMissingEpisodes, dir, tvdbid, url) VALUES(:hide, :dir, :tvdbid, :url)");
        query.bindValue(":dir", show->dir().toUtf8());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.bindValue(":tvdbid", show->tvdbId().isEmpty() ? "" : show->tvdbId());
        query.bindValue(":hide", hideSpecials ? 1 : 0);
        query.exec();
    }
}

void Database::add(TvShowEpisode *episode, QString path, int idShow)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO episodes(content, idShow, path, seasonNumber, episodeNumber) "
                  "VALUES(:content, :idShow, :path, :seasonNumber, :episodeNumber)");
    query.bindValue(":content", episode->nfoContent().isEmpty() ? "" : episode->nfoContent().toUtf8());
    query.bindValue(":idShow", idShow);
    query.bindValue(":path", path.toUtf8());
    query.bindValue(":seasonNumber", episode->season());
    query.bindValue(":episodeNumber", episode->episode());
    query.exec();
    int insertId = query.lastInsertId().toInt();
    foreach (const QString &file, episode->files()) {
        query.prepare("INSERT INTO episodeFiles(idEpisode, file) VALUES(:idEpisode, :file)");
        query.bindValue(":idEpisode", insertId);
        query.bindValue(":file", file.toUtf8());
        query.exec();
    }
    episode->setDatabaseId(insertId);
}

void Database::update(TvShow *show)
{
    QSqlQuery query(db());
    query.prepare("UPDATE shows SET content=:content WHERE idShow=:id");
    query.bindValue(":content", show->nfoContent().isEmpty() ? "" : show->nfoContent());
    query.bindValue(":id", show->databaseId());
    query.exec();

    int id = showsSettingsId(show);
    query.prepare("UPDATE showsSettings SET showMissingEpisodes=:show, hideSpecialsInMissingEpisodes=:hide, url=:url, tvdbid=:tvdbid WHERE idShow=:idShow");
    query.bindValue(":show", show->showMissingEpisodes());
    query.bindValue(":hide", show->hideSpecialsInMissingEpisodes());
    query.bindValue(":idShow", id);
    query.bindValue(":tvdbid", show->tvdbId().isEmpty() ? "" : show->tvdbId());
    query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
    query.exec();
}

void Database::update(TvShowEpisode *episode)
{
    QSqlQuery query(db());
    query.prepare("UPDATE episodes SET content=:content WHERE idEpisode=:id");
    query.bindValue(":content", episode->nfoContent().isEmpty() ? "" : episode->nfoContent());
    query.bindValue(":id", episode->databaseId());
    query.exec();
}

QList<TvShow*> Database::shows(QString path)
{
    QList<TvShow*> shows;
    QSqlQuery query(db());
    query.prepare("SELECT idShow, dir, content, path FROM shows WHERE path=:path");
    query.bindValue(":path", path.toUtf8());
    query.exec();
    while (query.next()) {
        TvShow *show = new TvShow(QString::fromUtf8(query.value(query.record().indexOf("dir")).toByteArray()), Manager::instance()->tvShowFileSearcher());
        show->setDatabaseId(query.value(query.record().indexOf("idShow")).toInt());
        show->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        shows.append(show);
    }

    foreach (TvShow *show, shows) {
        query.prepare("SELECT showMissingEpisodes, hideSpecialsInMissingEpisodes FROM showsSettings WHERE dir=:dir");
        query.bindValue(":dir", show->dir().toUtf8());
        query.exec();
        if (query.next()) {
            show->setShowMissingEpisodes(query.value(query.record().indexOf("showMissingEpisodes")).toInt() == 1, false);
            show->setHideSpecialsInMissingEpisodes(query.value(query.record().indexOf("hideSpecialsInMissingEpisodes")).toInt() == 1, false);
        }
    }

    return shows;
}

QList<TvShowEpisode*> Database::episodes(int idShow)
{
    QList<TvShowEpisode*> episodes;
    QSqlQuery query(db());
    QSqlQuery queryFiles(db());
    query.prepare("SELECT idEpisode, content, seasonNumber, episodeNumber FROM episodes WHERE idShow=:idShow");
    query.bindValue(":idShow", idShow);
    query.exec();
    while (query.next()) {
        QStringList files;
        queryFiles.prepare("SELECT file FROM episodeFiles WHERE idEpisode=:idEpisode");
        queryFiles.bindValue(":idEpisode", query.value(query.record().indexOf("idEpisode")).toInt());
        queryFiles.exec();
        while (queryFiles.next())
            files << QString::fromUtf8(queryFiles.value(queryFiles.record().indexOf("file")).toByteArray());

        TvShowEpisode *episode = new TvShowEpisode(files);
        episode->setSeason(query.value(query.record().indexOf("seasonNumber")).toInt());
        episode->setEpisode(query.value(query.record().indexOf("episodeNumber")).toInt());
        episode->setDatabaseId(query.value(query.record().indexOf("idEpisode")).toInt());
        episode->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        episodes.append(episode);
    }
    return episodes;
}

void Database::clearTvShows(QString path)
{
    QSqlQuery query(db());
    if (!path.isEmpty()) {
        query.prepare("DELETE FROM shows WHERE path=:path");
        query.bindValue(":path", path.toUtf8());
        query.exec();
        query.prepare("DELETE FROM episodeFiles WHERE idEpisode IN (SELECT idEpisode FROM episodes WHERE path=:path)");
        query.bindValue(":path", path.toUtf8());
        query.exec();
        query.prepare("DELETE FROM episodes WHERE path=:path");
        query.bindValue(":path", path.toUtf8());
        query.exec();
    } else {
        query.prepare("DELETE FROM shows");
        query.exec();
        query.prepare("DELETE FROM episodes");
        query.exec();
        query.prepare("DELETE FROM episodeFiles");
        query.exec();
        query.prepare("DELETE FROM sqlite_sequence WHERE name='shows'");
        query.exec();
        query.prepare("DELETE FROM sqlite_sequence WHERE name='episodes'");
        query.exec();
        query.prepare("DELETE FROM sqlite_sequence WHERE name='episodeFiles'");
        query.exec();
    }
}

void Database::clearTvShow(QString showDir)
{
    QSqlQuery query(db());
    query.prepare("SELECT idShow FROM shows WHERE dir=:dir");
    query.bindValue(":dir", showDir.toUtf8());
    query.exec();
    if (!query.next())
        return;
    int idShow = query.value(0).toInt();

    query.prepare("DELETE FROM episodeFiles WHERE idEpisode IN (SELECT idEpisode FROM episodes WHERE idShow=:idShow)");
    query.bindValue(":idShow", idShow);
    query.exec();

    query.prepare("DELETE FROM shows WHERE idShow=:idShow");
    query.bindValue(":idShow", idShow);
    query.exec();

    query.prepare("DELETE FROM episodes WHERE idShow=:idShow");
    query.bindValue(":idShow", idShow);
    query.exec();
}

int Database::episodeCount()
{
    QSqlQuery query(db());
    query.prepare("SELECT COUNT(*) FROM episodes");
    query.exec();
    query.next();
    return query.value(0).toInt();
}

int Database::showsSettingsId(TvShow *show)
{
    QSqlQuery query(db());
    query.prepare("SELECT idShow FROM showsSettings WHERE dir=:dir");
    query.bindValue(":dir", show->dir().toUtf8());
    query.exec();
    if (query.next())
        return query.value(0).toInt();

    query.prepare("INSERT INTO showsSettings(showMissingEpisodes, hideSpecialsInMissingEpisodes, dir) VALUES(:show, :hide, :dir)");
    query.bindValue(":dir", show->dir().toUtf8());
    query.bindValue(":show", 0);
    query.bindValue(":hide", 0);
    query.exec();
    return query.lastInsertId().toInt();
}

void Database::clearEpisodeList(int showsSettingsId)
{
    QSqlQuery query(db());
    query.prepare("UPDATE showsEpisodes SET updated=0 WHERE idShow=:idShow");
    query.bindValue(":idShow", showsSettingsId);
    query.exec();
}

void Database::addEpisodeToShowList(TvShowEpisode *episode, int showsSettingsId, QString tvdbid)
{
    QByteArray xmlContent;
    QXmlStreamWriter xmlWriter(&xmlContent);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument("1.0", true);
    XbmcXml::writeTvShowEpisodeXml(xmlWriter, episode);
    xmlWriter.writeEndDocument();

    QSqlQuery query(db());
    query.prepare("SELECT idEpisode FROM showsEpisodes WHERE tvdbid=:tvdbid");
    query.bindValue(":tvdbid", tvdbid);
    query.exec();
    if (query.next()) {
        int idEpisode = query.value(0).toInt();
        query.prepare("UPDATE showsEpisodes SET seasonNumber=:seasonNumber, episodeNumber=:episodeNumber, updated=1, content=:content WHERE idEpisode=:idEpisode");
        query.bindValue(":content", xmlContent.isEmpty() ? "" : xmlContent);
        query.bindValue(":idEpisode", idEpisode);
        query.bindValue(":seasonNumber", episode->season());
        query.bindValue(":episodeNumber", episode->episode());
        query.exec();
    } else {
        query.prepare("INSERT INTO showsEpisodes(content, idShow, seasonNumber, episodeNumber, tvdbid, updated) "
                      "VALUES(:content, :idShow, :seasonNumber, :episodeNumber, :tvdbid, 1)");
        query.bindValue(":content", xmlContent.isEmpty() ? "" : xmlContent);
        query.bindValue(":idShow", showsSettingsId);
        query.bindValue(":seasonNumber", episode->season());
        query.bindValue(":episodeNumber", episode->episode());
        query.bindValue(":tvdbid", tvdbid);
        query.exec();
    }
}

void Database::cleanUpEpisodeList(int showsSettingsId)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM showsEpisodes WHERE idShow=:idShow AND updated=0");
    query.bindValue(":idShow", showsSettingsId);
    query.exec();
}

QList<TvShowEpisode*> Database::showsEpisodes(TvShow *show)
{
    int id = showsSettingsId(show);
    QList<TvShowEpisode*> episodes;
    QSqlQuery query(db());
    query.prepare("SELECT idEpisode, content, seasonNumber, episodeNumber FROM showsEpisodes WHERE idShow=:idShow");
    query.bindValue(":idShow", id);
    query.exec();
    while (query.next()) {
        TvShowEpisode *episode = new TvShowEpisode(QStringList(), show);
        episode->setSeason(query.value(query.record().indexOf("seasonNumber")).toInt());
        episode->setEpisode(query.value(query.record().indexOf("episodeNumber")).toInt());
        episode->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        episodes.append(episode);
    }
    return episodes;
}

void Database::addImport(QString fileName, QString type, QString path)
{
    int id = 1;
    QSqlQuery query(db());
    query.prepare("SELECT MAX(id) FROM importCache");
    query.exec();
    if (query.next())
        id = query.value(0).toInt()+1;

    query.prepare("INSERT INTO importCache(id, filename, type, path) VALUES(:id, :filename, :type, :path)");
    query.bindValue(":id", id);
    query.bindValue(":filename", fileName);
    query.bindValue(":type", type);
    query.bindValue(":path", path);
    query.exec();
}

bool Database::guessImport(QString fileName, QString &type, QString &path)
{
    qreal bestMatch = 0;

    QSqlQuery query(db());
    query.prepare("SELECT filename, type, path FROM importCache");
    query.exec();
    while (query.next()) {
        qreal p = Helper::similarity(fileName, query.value(query.record().indexOf("filename")).toString());
        if (p > 0.7 && p > bestMatch) {
            bestMatch = p;
            type = query.value(query.record().indexOf("type")).toString();
            path = query.value(query.record().indexOf("path")).toString();
        }
    }

    return (bestMatch != 0);
}

void Database::setLabel(QStringList fileNames, int color)
{
    QSqlQuery query(db());
    int id = 1;
    query.prepare("SELECT MAX(idLabel) FROM labels");
    query.exec();
    if (query.next())
        id = query.value(0).toInt()+1;

    foreach (const QString &fileName, fileNames) {
        query.prepare("SELECT idLabel FROM labels WHERE fileName=:fileName");
        query.bindValue(":fileName", fileName.toUtf8());
        query.exec();
        if (query.next()) {
            int idLabel = query.value(query.record().indexOf("idLabel")).toInt();
            query.prepare("UPDATE labels SET color=:color WHERE idLabel=:idLabel");
            query.bindValue(":idLabel", idLabel);
            query.bindValue(":color", color);
            query.exec();
        } else {
            query.prepare("INSERT INTO labels(idLabel, color, fileName) VALUES(:idLabel, :color, :fileName)");
            query.bindValue(":idLabel", id);
            query.bindValue(":color", color);
            query.bindValue(":fileName", fileName.toUtf8());
            query.exec();
        }
    }
}

int Database::getLabel(QStringList fileNames)
{
    if (fileNames.isEmpty())
        return Labels::NO_LABEL;

    QSqlQuery query(db());
    query.prepare("SELECT color FROM labels WHERE fileName=:fileName");
    query.bindValue(":fileName", fileNames.first().toUtf8());
    query.exec();
    if (query.next())
        return query.value(query.record().indexOf("color")).toInt();

    return Labels::NO_LABEL;
}
