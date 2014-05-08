/*
 * marchingsquares.cpp
 *
 *  Created on: May 6, 2014
 *      Author: schurade
 */

#include "marchingsquares.h"

MarchingSquares::MarchingSquares( std::vector<float>* data, float isoValue, int nx, int ny, float dx, float dy ) :
    m_data( data ),
    m_isoValue( isoValue ),
    m_nx( nx ),
    m_ny( ny ),
    m_dx( dx ),
    m_dy( dy ),
    m_dx2( dx / 2 ),
    m_dy2( dy / 2 )
{
}

MarchingSquares::~MarchingSquares()
{
}

int MarchingSquares::id( int x, int y )
{
    return x + m_nx * y;
}

int MarchingSquares::step( int x, int y )
{
    // Scan our 4 pixel area
    bool upLeft = m_mask[ id( x-1, y-1 ) ];
    bool upRight = m_mask[ id( x, y-1 ) ];
    bool downLeft = m_mask[ id( x-1, y ) ];
    bool downRight = m_mask[ id( x, y ) ];

    // Determine which state we are in
    int state = 0;

    if ( downLeft )
    {
        state |= 1;
    }
    if ( downRight )
    {
        state |= 2;
    }
    if ( upRight )
    {
        state |= 4;
    }
    if ( upLeft )
    {
        state |= 8;
    }

    return state;
}

std::vector<float> MarchingSquares::run()
{
    m_verts.clear();
    m_mask.resize( m_data->size(), false );
    for ( unsigned int i = 0; i < m_data->size(); ++i )
    {
        if ( m_data->at( i ) > m_isoValue )
        {
            m_mask[i] = true;
        }
    }

    m_states.resize( m_nx * m_ny, 0 );

    for ( int y = 1; y < m_ny; ++y )
    {
        for ( int x = 1; x < m_nx; ++x )
        {
            m_states[ id( x, y ) ] = step( x, y );
        }
    }

    for ( int y = 1; y < m_ny; ++y )
    {
        for ( int x = 1; x < m_nx; ++x )
        {
            int state = m_states[ id( x, y ) ];

            switch ( state )
            {
                case 0:
                case 15:
                    break;
                case 1:
                    paintSW( x-1, y-1 );
                    break;
                case 2:
                    paintSE( x - 1, y - 1 );
                    break;
                case 3:
                    paintEW( x - 1, y - 1 );
                    break;
                case 4:
                    paintNE( x - 1, y - 1 );
                    break;
                case 5:
                    paintSE( x - 1, y - 1 );
                    paintNW( x - 1, y - 1 );
                    break;
                case 6:
                    paintSN( x - 1, y - 1 );
                    break;
                case 7:
                    paintNW( x - 1, y - 1 );
                    break;
                case 8:
                    paintNW( x - 1, y - 1 );
                    break;
                case 9:
                    paintSN( x - 1, y - 1 );
                    break;
                case 10:
                    paintSW( x - 1, y - 1 );
                    paintNE( x - 1, y - 1 );
                    break;
                case 11:
                    paintNE( x - 1, y - 1 );
                    break;
                case 12:
                    paintEW( x - 1, y - 1 );
                    break;
                case 13:
                    paintSE( x - 1, y - 1 );
                    break;
                case 14:
                    paintSW( x - 1, y - 1 );
                    break;
            }
        }
    }
    return m_verts;
}

void MarchingSquares::paintNW( int x, int y )
{
    m_verts.push_back( x * m_dx );
    m_verts.push_back( y * m_dy + m_dy2 );
    m_verts.push_back( 0.0 );
    m_verts.push_back( x * m_dx + m_dx2 );
    m_verts.push_back( y * m_dy );
    m_verts.push_back( 0.0 );
}

void MarchingSquares::paintNE( int x, int y )
{
    m_verts.push_back( x * m_dx + m_dx2 );
    m_verts.push_back( y * m_dy );
    m_verts.push_back( 0.0 );
    m_verts.push_back( x * m_dx + m_dx );
    m_verts.push_back( y * m_dy + m_dy2 );
    m_verts.push_back( 0.0 );
}

void MarchingSquares::paintSE( int x, int y )
{
    m_verts.push_back( x * m_dx + m_dx2 );
    m_verts.push_back( y * m_dy + m_dy );
    m_verts.push_back( 0.0 );
    m_verts.push_back( x * m_dx + m_dx );
    m_verts.push_back( y * m_dy + m_dy2 );
    m_verts.push_back( 0.0 );
}

void MarchingSquares::paintSW( int x, int y )
{
    m_verts.push_back( x * m_dx );
    m_verts.push_back( y * m_dy + m_dy2 );
    m_verts.push_back( 0.0 );
    m_verts.push_back( x * m_dx + m_dx2 );
    m_verts.push_back( y * m_dy + m_dy );
    m_verts.push_back( 0.0 );
}

void MarchingSquares::paintSN( int x, int y )
{
    m_verts.push_back( x * m_dx + m_dx2 );
    m_verts.push_back( y * m_dy );
    m_verts.push_back( 0.0 );
    m_verts.push_back( x * m_dx + m_dx2 );
    m_verts.push_back( y * m_dy + m_dy );
    m_verts.push_back( 0.0 );
}

void MarchingSquares::paintEW( int x, int y )
{
    m_verts.push_back( x * m_dx );
    m_verts.push_back( y * m_dy + m_dy2 );
    m_verts.push_back( 0.0 );
    m_verts.push_back( x * m_dx + m_dx );
    m_verts.push_back( y * m_dy + m_dy2 );
    m_verts.push_back( 0.0 );
}
