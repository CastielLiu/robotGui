#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <QObject>
#include <QFile>
#include <QDebug>
#include <iostream>
#include <QXmlStreamReader>

#pragma pack(push,1)
typedef struct Position
{
    double x,y;
} position_t;

typedef struct Orientation
{
    double x,y,z,w;
} orientation_t;

typedef struct Pose
{
    position_t position;
    orientation_t orientation;
} pose_t;

#pragma pack(pop)

typedef struct GoalInfo
{
    int id;
    bool validity;
    std::string name;
    pose_t pose;

    GoalInfo()
    {
        validity = false;
    }

    void print() const
    {
        std::cout << "id: " << id << "\tname:" << name  << std::endl;
        std::cout << "position: " << pose.position.x << "\t" << pose.position.y << "\t" << std::endl;
        std::cout << "orientation: " << pose.orientation.x << "\t" << pose.orientation.y
                  << "\t" << pose.orientation.z << "\t" << pose.orientation.w << std::endl << std::endl;
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
