/*
 * roipropertywidget.cpp
 *
 * Created on: 03.02.2013
 * @author Ralph Schurade
 */
#include "roipropertywidget.h"

#include "../views/roipropertyview.h"

#include "../widgets/controls/sliderwithedit.h"
#include "../widgets/controls/sliderwitheditint.h"
#include "../widgets/controls/selectwithlabel.h"
#include "../widgets/controls/checkbox.h"

#include "../../data/enums.h"
#include "../../data/vptr.h"
#include "../../data/roi.h"
#include "../../data/properties/property.h"

#include <QtWidgets>

ROIPropertyWidget::ROIPropertyWidget( QWidget* parent ) :
    QTabWidget( parent ),
    m_buildingTabs( false )
{
    m_propertyView = new ROIPropertyView( this );

    m_layout = new QVBoxLayout();
    m_layout->setContentsMargins( 1, 1, 1, 1 );
    m_layout->setSpacing( 1 );
    m_layout->addStretch();
    setLayout( m_layout );

    setContentsMargins( 0, 0, 0, 0 );

    connect( m_propertyView, SIGNAL( selectedChanged() ), this, SLOT( updateWidgetVisibility() ) );
    connect( m_propertyView, SIGNAL( nothingSelected() ), this, SLOT( clearWidget() ) );
    connect( this, SIGNAL( currentChanged( int ) ), this, SLOT( slotTabChanged( int ) ) );
}

ROIPropertyWidget::~ROIPropertyWidget()
{
}

void ROIPropertyWidget::setModel( QAbstractItemModel* model )
{
    m_propertyView->setModel( model );
}

void ROIPropertyWidget::setSelectionModel( QItemSelectionModel* selectionModel )
{
    m_propertyView->setSelectionModel( selectionModel );
}

void ROIPropertyWidget::updateWidgetVisibility()
{
    m_buildingTabs = true;
    // clear tabs
    while ( count() > 0 )
    {
        removeTab( 0 );
    }
    repaint();
    // get properties
    QModelIndex index = m_propertyView->getSelectedIndex( (int)Fn::Property::D_POINTER );
    ROI* roi = VPtr<ROI>::asPtr( m_propertyView->model()->data( index, Qt::DisplayRole ) );

    PropertyGroup* props = roi->properties();
    QHash<QString, QVBoxLayout*>tabs;

    for ( int i = 0; i < props->size(); ++i )
    {
        // check tab
        QString tab = props->getNthProperty( i )->getPropertyTab();
        if ( tab == "none" )
        {
            continue;
        }
        //create tab if not exists
        if ( !tabs.contains( tab ) )
        {
            QVBoxLayout* layout = new QVBoxLayout();
            layout->setContentsMargins( 1, 1, 1, 1 );
            layout->setSpacing( 1 );

            QWidget* widget = new QWidget;
            widget->setLayout( layout );
            widget->setContentsMargins( 0, 0, 0, 0 );

            QScrollArea* scrollArea = new QScrollArea( this );
            scrollArea->setWidgetResizable( true );
            scrollArea->setWidget( widget );

            addTab( scrollArea, tab );
            tabs[tab] = layout;
        }
        tabs[tab]->addWidget( props->getNthProperty( i )->getWidget() );
    }

    QHashIterator<QString, QVBoxLayout*> ti( tabs );
    while ( ti.hasNext() )
    {
        ti.next();
        ti.value()->addStretch();
    }
    for ( int i = 0; i < count(); ++i )
    {
        if ( m_lastUsedTab == tabText( i ) )
        {
            setCurrentIndex( i );
            break;
        }
    }
    m_buildingTabs = false;
}

void ROIPropertyWidget::clearWidget()
{
    for ( int i = 0; i < m_visibleWidgets.size(); ++i )
    {
        m_visibleWidgets[i]->hide();
        m_layout->removeWidget( m_visibleWidgets[i] );
    }
    m_layout->removeItem( m_layout->itemAt( 0 ) );
    m_visibleWidgets.clear();
    repaint();

    m_layout->addStretch();
}

void ROIPropertyWidget::slotTabChanged( int tab )
{
    if( m_buildingTabs )
    {
        return;
    }
    m_lastUsedTab = tabText( tab );
}
