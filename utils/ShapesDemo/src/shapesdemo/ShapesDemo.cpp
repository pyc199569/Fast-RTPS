/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#include <iostream>
#include <sstream>

#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"
#include "eprosimashapesdemo/shapesdemo/ShapePublisher.h"
#include "eprosimashapesdemo/shapesdemo/ShapeSubscriber.h"
#include "eprosimashapesdemo/shapesdemo/Shape.h"
#include "eprosimashapesdemo/qt/mainwindow.h"

ShapesDemo::ShapesDemo(MainWindow *mw):
    mp_RTPSParticipant(NULL),
    m_isInitialized(false),
    minX(0),minY(0),maxX(0),maxY(0),
    m_mainWindow(mw),
    m_mutex(QMutex::Recursive)
{
    srand (time(NULL));
    minX = 0;
    minY = 0;
    maxX = MAX_DRAW_AREA_X;
    maxY = MAX_DRAW_AREA_Y;
}

ShapesDemo::~ShapesDemo()
{
    stop();
}

RTPSParticipant* ShapesDemo::getRTPSParticipant()
{
    if(m_isInitialized && mp_RTPSParticipant !=NULL)
        return mp_RTPSParticipant;
    else
    {
        if(init())
            return mp_RTPSParticipant;
    }
    return NULL;
}

bool ShapesDemo::init()
{
    cout << "Initializing ShapesDemo "<< m_isInitialized <<endl;
    if(!m_isInitialized)
    {
        cout <<"Creating new RTPSParticipant"<<endl;
        RTPSParticipantAttributes pparam;
        pparam.name = "fastrtpsParticipant";
        pparam.builtin.domainId = m_options.m_domainId;
        pparam.builtin.leaseDuration.seconds = 100;
        pparam.builtin.leaseDuration_announcementperiod.seconds = 50;
        pparam.defaultSendPort = 10042;
        pparam.sendSocketBufferSize = 65536;
        pparam.listenSocketBufferSize = 2*65536;
        mp_RTPSParticipant = RTPSDomain::createRTPSParticipant(pparam);
        if(mp_RTPSParticipant!=NULL)
        {
            // cout << "RTPSParticipant Created "<< mp_RTPSParticipant->getGuid() << endl;l
            m_isInitialized = true;
            RTPSDomain::registerType(&m_shapeTopicDataType);
            return true;
        }
        return false;
    }
    return true;
}

void ShapesDemo::stop()
{
    if(m_isInitialized)
    {
        QMutexLocker lock(&m_mutex);
        this->m_mainWindow->quitThreads();
        for(std::vector<ShapePublisher*>::iterator it = m_publishers.begin();
            it!=m_publishers.end();++it)
        {
            delete(*it);
        }
        m_publishers.clear();
        for(std::vector<ShapeSubscriber*>::iterator it = m_subscribers.begin();
            it!=m_subscribers.end();++it)
        {
            delete(*it);
        }
        m_subscribers.clear();
        RTPSDomain::removeRTPSParticipant(mp_RTPSParticipant);
        cout << "All Stoped, removing"<<endl;
        mp_RTPSParticipant = NULL;
        m_isInitialized = false;
    }
}

void ShapesDemo::addPublisher(ShapePublisher* SP)
{
    m_publishers.push_back(SP);
    this->m_mainWindow->addPublisherToTable(SP);
}

void ShapesDemo::addSubscriber(ShapeSubscriber* SSub)
{
    m_subscribers.push_back(SSub);
    this->m_mainWindow->addSubscriberToTable(SSub);
}

uint32_t ShapesDemo::getRandomX(uint32_t size)
{
    return minX+size+(uint32_t)(((maxX-size)-(minX+size))*((double) rand() / (RAND_MAX)));
}

uint32_t ShapesDemo::getRandomY(uint32_t size)
{
    return minY+size+(uint32_t)(((maxY-size)-(minY+size))*((double) rand() / (RAND_MAX)));
}

void ShapesDemo::moveAllShapes()
{
    for(std::vector<ShapePublisher*>::iterator it = m_publishers.begin();
        it!=m_publishers.end();++it)
    {
        (*it)->m_mutex.lock();
        moveShape(&(*it)->m_shape);
        (*it)->m_mutex.unlock();
    }
}

void ShapesDemo::moveShape(Shape* sh)
{
    if(sh->m_changeDir)
        getNewDirection(sh);
    //Apply movement
    int nx = sh->m_x + m_options.m_movementSpeed*sh->m_dirX;
    int ny = sh->m_y + m_options.m_movementSpeed*sh->m_dirY;
    //Check if the movement is correct
    bool cond1 = nx+(int)sh->m_size/2 > (int)maxX;
    bool cond2 = nx-(int)sh->m_size/2 < (int)minX;
    bool cond3 = ny+(int)sh->m_size/2 > (int)maxY;
    bool cond4 = ny-(int)sh->m_size/2 < (int)minY;
    while(cond1 || cond2 || cond3 || cond4)
    {
        getNewDirection(sh);
        nx = sh->m_x + m_options.m_movementSpeed*sh->m_dirX;
        ny = sh->m_y + m_options.m_movementSpeed*sh->m_dirY;
        cond1 = nx+(int)sh->m_size/2 > (int)maxX;
        cond2 = nx-(int)sh->m_size/2 < (int)minX;
        cond3 = ny+(int)sh->m_size/2 > (int)maxY;
        cond4 = ny-(int)sh->m_size/2 < (int)minY;
    }
    sh->m_x = nx;
    sh->m_y = ny;
}

void ShapesDemo::getNewDirection(Shape* sh)
{
    sh->m_dirX = ((double) rand() / (RAND_MAX))*2-1;
    sh->m_dirY = ((double) rand() / (RAND_MAX))*2-1;
    //Normalize
    float module = sqrt(pow(sh->m_dirX,2)+pow(sh->m_dirY,2));
    sh->m_dirX /= module;
    sh->m_dirY /= module;
    sh->m_changeDir = false;
}


void ShapesDemo::writeAll()
{
    for(std::vector<ShapePublisher*>::iterator it = m_publishers.begin();
        it!=m_publishers.end();++it)
    {
        (*it)->write();
    }
}

void ShapesDemo::setOptions(ShapesDemoOptions& opt)
{
    m_options = opt;
    m_mainWindow->updateInterval(m_options.m_updateIntervalMs);

}

ShapesDemoOptions ShapesDemo::getOptions()
{

    return m_options;
}

void ShapesDemo::removePublisher(ShapePublisher* SP)
{
    cout << "REMOVING PUBLISHER"<<endl;
    for(std::vector<ShapePublisher*>::iterator it = this->m_publishers.begin();
        it!=this->m_publishers.end();++it)
    {
        if(SP->mp_pub->getGuid() == (*it)->mp_pub->getGuid())
        {
            m_publishers.erase(it);
            delete(SP);
            break;
        }
    }

}

void ShapesDemo::removeSubscriber(ShapeSubscriber* SS)
{
    cout << "REMOVING SUBSCRIBER"<<endl;
    for(std::vector<ShapeSubscriber*>::iterator it = this->m_subscribers.begin();
        it!=this->m_subscribers.end();++it)
    {
        if(SS->mp_sub->getGuid() == (*it)->mp_sub->getGuid())
        {
            m_subscribers.erase(it);
            delete(SS);
            break;
        }
    }
}
