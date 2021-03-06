#include "DataManager.h"

DataManager *DataManager::m_instance = nullptr;

DataManager::DataManager(QObject *parent)
    : QObject(parent)
    , m_lat(DEFAULT_LAT)
    , m_lng(DEFAULT_LNG)
    , m_temp(DEFAULT_TEMP)
    , m_humi(DEFAULT_HUMI)
    , m_rain(false)
    , m_dust("0")
{
    createDatabase();
    initDb();
}

DataManager* DataManager::getInstance()
{
    if(m_instance == nullptr)
        m_instance = new DataManager();
    return m_instance;
}

QString DataManager::temp() const
{
    return m_temp;
}

void DataManager::setTemp(const QString &temp)
{
    if(m_temp != temp)
        m_temp = temp;
}

QString DataManager::humi() const
{
    return m_humi;
}

void DataManager::setHumi(const QString &humi)
{
    if(m_humi != humi)
        m_humi = humi;
}

QString DataManager::lat() const
{
    return m_lat;
}

void DataManager::setLat(const QString &lat)
{
    if(m_lat != lat)
        m_lat = lat;
}

QString DataManager::lng() const
{
    return m_lng;
}

void DataManager::setLng(const QString &lng)
{
    if(m_lng != lng)
        m_lng = lng;
}

QString DataManager::dust() const
{
    return m_dust;
}

void DataManager::setDust(const QString &dust)
{
    if(m_dust != dust)
        m_dust = dust;
}

bool DataManager::rain() const
{
    return m_rain;
}

void DataManager::setRain(bool rain)
{
    if(m_rain != rain)
        m_rain = rain;
}

bool DataManager::createDatabase()
{
    if (!QSqlDatabase::drivers().contains("QSQLITE")) {
        DLOG_THREAD << "Unable to load database, needs install SQLITE driver";
    }
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    bool success = false;
    database.setDatabaseName(PH_DB);
    if (database.open()) {
        DLOG_THREAD << "Database created/registered.";
        success = true;
    } else {
        DLOG_THREAD << "Error opening connection to the database: " << database.lastError().text();
    }
    database.close();
    return success;
}

void DataManager::initDb()
{
    if(isDbExists()) {
        QSqlDatabase database = QSqlDatabase::database();
        if (!database.tables().contains("HistoryData")) {
            createHistoryDataTable();
        }
    }
}

bool DataManager::isDbExists()
{
    return QFile::exists(PH_DB);
}

void DataManager::createHistoryDataTable()
{
    QSqlDatabase database = QSqlDatabase::database();
    const QString createSQL = "CREATE TABLE IF NOT EXISTS HistoryData("
                              "time DATETIME NOT NULL,"
                              "lat TEXT,"
                              "lng TEXT,"
                              "temp TEXT,"
                              "humi TEXT,"
                              "rain BOOLEAN,"
                              "dust TEXT);";
    QSqlQuery query(database);
    if (query.exec(createSQL)) {
        DLOG_THREAD << "Table creation query execute successfully";
    } else {
        DLOG_THREAD << "Create table error: " << database.lastError().text();
    }
    database.close();
}

void DataManager::addHistoryDataRow(QString time, QString lat, QString lng, QString temp, QString humi, bool rain, QString dust)
{
    QSqlDatabase database = QSqlDatabase::database();
    if (!database.tables().contains("HistoryData")) {
        DLOG_THREAD << "Create record error: Accounts table does not exist.";
    } else {
        QSqlQuery query(database);
        query.prepare("INSERT INTO HistoryData(time, lat, lng, temp, humi, rain, dust) "
                      "VALUES (:time, :lat, :lng, :temp, :humi, :rain, :dust);");
        query.bindValue(":time", time);
        query.bindValue(":lat", lat);
        query.bindValue(":lng", lng);
        query.bindValue(":temp", temp);
        query.bindValue(":humi", humi);
        query.bindValue(":rain", rain);
        query.bindValue(":dust", dust);
        if (query.exec()) {
            DLOG_THREAD <<  "Insert done";
        } else {
            DLOG_THREAD << "Insert error: " << database.lastError().text();
        }
    }
    database.close();
}

QString DataManager::getAlert()
{
    QString retVal = "0000";
    int tempLevel, humiLevel, rainLevel, dustLevel;

    int tempVal = m_temp.toInt();
    int humiVal = m_humi.toInt();
    double dustVal = m_dust.toDouble();

    if(tempVal < 16)
        tempLevel = TEMP_LOW;
    else if(tempVal > 30)
        tempLevel = TEMP_HIGH;
    else
        tempLevel = TEMP_NORMAL;

    if(humiVal < 60)
        humiLevel = HUMI_LOW;
    else if(humiVal > 80)
        humiLevel = HUMI_HIGH;
    else
        humiLevel = HUMI_NORMAL;

    rainLevel = m_rain ? RAIN_RAINING : RAIN_NO_RAIN;

    if(dustVal < 35)
        dustLevel = DUST_LV1_EXCELLENT;
    else if(dustVal >= 35 && dustVal < 75)
        dustLevel = DUST_LV2_AVERAGE;
    else if(dustVal >= 75 && dustVal < 115)
        dustLevel = DUST_LV3_LIGHT_POLLUTION;
    else if(dustVal >= 115 && dustVal < 150)
        dustLevel = DUST_LV4_MODERATE_POLLUTION;
    else if(dustVal >= 150 && dustVal < 250)
        dustLevel = DUST_LV5_HEAVY_POLLUTION;
    else /*if(dustVal >= 250)*/
        dustLevel = DUST_LV6_SERIOUS_POLLUTION;

    return QString("%1%2%3%4").arg(tempLevel, humiLevel, rainLevel, dustLevel);
}

