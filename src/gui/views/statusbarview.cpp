/*
 * statusbarview.cpp
 *
 * Created on: Nov 13, 2012
 * @author Ralph Schurade
 */
#include "statusbarview.h"

#include "../../data/enums.h"
#include "../../data/datasets/dataset.h"
#include "../../data/vptr.h"
#include "../../data/models.h"

#include <QDebug>

StatusBarView::StatusBarView() :
    m_selected( 0 ),
    m_globalInfo( "" ),
    m_datasetInfo( "" ),
    m_x( 0 ),
    m_y( 0 ),
    m_z( 0 )
{
    connect( Models::g(), SIGNAL( dataChanged( QModelIndex, QModelIndex ) ), this, SLOT( dataChanged( QModelIndex, QModelIndex ) ) );
}

StatusBarView::~StatusBarView()
{
}

QRect StatusBarView::visualRect( const QModelIndex &index ) const
{
    return QRect( 0, 0, 0, 0 );
}

void StatusBarView::scrollTo( const QModelIndex &index, ScrollHint hint )
{
}

QModelIndex StatusBarView::indexAt( const QPoint &point ) const
{
    return QModelIndex();
}

QModelIndex StatusBarView::moveCursor( CursorAction cursorAction, Qt::KeyboardModifiers modifiers )
{
    return QModelIndex();
}

int StatusBarView::horizontalOffset() const
{
    return 0;
}

int StatusBarView::verticalOffset() const
{
    return 0;
}

bool StatusBarView::isIndexHidden( const QModelIndex &index ) const
{
    return false;
}

void StatusBarView::setSelection( const QRect &rect, QItemSelectionModel::SelectionFlags flags )
{
}

QRegion StatusBarView::visualRegionForSelection( const QItemSelection &selection ) const
{
    return QRegion();
}

void StatusBarView::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
    if ( selected.indexes().size() > 0 )
    {
        m_selected = selected.indexes().first().row();
        Dataset* ds = VPtr<Dataset>::asPtr( model()->data( model()->index( m_selected, (int)Fn::Property::D_DATASET_POINTER ), Qt::DisplayRole ) );
        m_datasetInfo = ds->getValueAsString( m_x, m_y, m_z );

        emit( sigStatusChanged( m_globalInfo + " " + m_datasetInfo ) );
    }
    else
    {
        m_selected = -1;
    }
}

void StatusBarView::dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
    if ( m_selected == -1 )
    {
        return;
    }
    m_globalInfo = "[";

    m_x = Models::getGlobal( Fn::Property::G_SAGITTAL ).toFloat();
    m_globalInfo +=  QString::number( m_x, 'f', 2 );
    m_globalInfo += ",";

    m_y = Models::getGlobal( Fn::Property::G_CORONAL ).toFloat();
    m_globalInfo += QString::number( m_y, 'f', 2 );
    m_globalInfo += ",";

    m_z = Models::getGlobal( Fn::Property::G_AXIAL ).toFloat();
    m_globalInfo += QString::number( m_z, 'f', 2 );
    m_globalInfo += "]";

    Dataset* ds = VPtr<Dataset>::asPtr( model()->data( model()->index( m_selected, (int)Fn::Property::D_DATASET_POINTER ), Qt::DisplayRole ) );
    if ( ds )
    {
        m_datasetInfo = ds->getValueAsString( m_x, m_y, m_z );
    }

    QString zoom = "[Zoom] " + QString::number( Models::zoom );

    emit( sigStatusChanged( m_globalInfo + " " + m_datasetInfo + " " + zoom ) );
}
