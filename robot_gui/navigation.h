#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <QObject>
#include <QFile>
#include <QDebug>
#include <iostream>
#include <QXmlStreamReader>


typedef struct Position
{
    double x,y,z;
} position_t;

typedef struct Orientation
{
    double x,y,z,w;
} orientation_t;

typedef struct GoalInfo
{
    int id;
    int seq;
    bool validity;
    std::string name;
    position_t position;
    orientation_t orientation;

    GoalInfo()
    {
        validity = false;
    }

    void print() const
    {
        std::cout << "id: " << id << "\tname:" << name << "\tseq" << seq << std::endl;
        std::cout << "position: " << position.x << "\t" << position.y << "\t" << position.z << std::endl;
        std::cout << "orientation: " << orientation.x << "\t" << orientation.y << "\t" << orientation.z
                  << "\t" << orientation.w << std::endl << std::endl;
    }

} goalInfo_t;


class Navigation : public QObject
{
    Q_OBJECT
public:
    explicit Navigation(QObject *parent = nullptr);
    bool loadGoalPointsInfo(const QString& file_name);
    goalInfo_t operator[](int index) const;
    goalInfo_t getGoalInfoByName(const std::string& name) const;

private:
    std::vector<goalInfo_t> mGoalsInfo;

signals:
    void updateGoals(std::vector<goalInfo_t> goalsInfo);

};

#endif // NAVIGATION_H
