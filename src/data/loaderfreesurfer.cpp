/*
 * loaderfreesurfer.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: boettgerj
 */

#include "loaderfreesurfer.h"

#include <QFile>
#include <QStringList>
#include <QTextStream>

LoaderFreesurfer::LoaderFreesurfer()
{

}

LoaderFreesurfer::~LoaderFreesurfer()
{
    // TODO Auto-generated destructor stub
}

QVector<float> LoaderFreesurfer::getPoints()
{
    return m_points;
}

QVector<int> LoaderFreesurfer::getTriangles()
{
    return m_triangles;
}

bool LoaderFreesurfer::loadASC( QString fn )
{
    QFile n( fn );
    if ( !n.open( QIODevice::ReadOnly ) )
    {
        qDebug( "nodes unreadable" );
        return false;
    }
    QTextStream ns( &n );

    QString nl;
    ns.readLine();
    nl = ns.readLine();
    int numPoints = nl.split( " " ).at( 0 ).toInt();
    int numTriangles = nl.split( " " ).at( 1 ).toInt();

    //POINTS
    for ( int i = 0; i < numPoints; i++ )
    {
        nl = ns.readLine();
        QStringList vals = nl.split( " ", QString::SkipEmptyParts );
        m_points << vals.at( 0 ).toFloat();
        m_points << vals.at( 1 ).toFloat();
        m_points << vals.at( 2 ).toFloat();
    }

    //TRIANGLES
    for ( int i = 0; i < numTriangles; i++ )
    {
        nl = ns.readLine();
        QStringList vals = nl.split( " ", QString::SkipEmptyParts );
        m_triangles << vals.at( 0 ).toInt();
        m_triangles << vals.at( 1 ).toInt();
        m_triangles << vals.at( 2 ).toInt();
    }
    return true;
}