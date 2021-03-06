/*
 * shrenderer.cpp
 *
 * Created on: 03.07.2012
 * @author Ralph Schurade
 */
#include "shrenderer.h"
#include "shrendererthread.h"
#include "glfunctions.h"

#include "../../data/datasets/datasetsh.h"
#include "../../data/enums.h"
#include "../../data/models.h"
#include "../../data/vptr.h"
#include "../../algos/fmath.h"
#include "../../algos/qball.h"

#include "../../data/mesh/tesselation.h"
#include "../../data/mesh/trianglemesh2.h"

#include "../../thirdparty/newmat10/newmat.h"

#include <QGLShaderProgram>
#include <QDebug>

#include <limits>

SHRenderer::SHRenderer( std::vector<ColumnVector>* data ) :
    ObjectRenderer(),
    m_tris( 0 ),
    vboIds( new GLuint[ 4 ] ),
    m_data( data ),
    m_scaling( 1.0 ),
    m_orient( 0 ),
    m_offset( 0 ),
    m_lod( 0 ),
    m_minMaxScaling( false ),
    m_order( 4 ),
    m_oldLoD( -1 ),
    m_pickId( GLFunctions::getPickIndex() ),
    m_updateThread( 0 ),
    m_glyphsUpdated( false ),
    m_updateRunning( false ),
    m_vertSize( 0 ),
    m_colorSize( 0 )
{
}

SHRenderer::~SHRenderer()
{
    glDeleteBuffers(1, &( vboIds[ 0 ] ) );
    glDeleteBuffers(1, &( vboIds[ 1 ] ) );
    glDeleteBuffers(1, &( vboIds[ 2 ] ) );
    glDeleteBuffers(1, &( vboIds[ 3 ] ) );
}

void SHRenderer::init()
{
    initializeOpenGLFunctions();
    glGenBuffers( 4, vboIds );
}

void SHRenderer::draw( QMatrix4x4 p_matrix, QMatrix4x4 mv_matrix, int width, int height, int renderMode, PropertyGroup& props )
{
    float alpha = 1.0; //props.get( Fn::Property::D_ALPHA ).toFloat();
    m_renderMode = renderMode;

    m_pMatrix = p_matrix;
    m_mvMatrix = mv_matrix;

    switch ( renderMode )
    {
        case 0:
            break;
        case 1:
        {
            if ( alpha < 1.0 ) // obviously not opaque
            {
                return;
            }
            break;
        }
        default:
        {
            if ( alpha == 1.0  ) // not transparent
            {
                return;
            }
            break;
        }
    }

    setRenderParams( props );

    if ( m_orient == 0 )
    {
        return;
    }

    QGLShaderProgram* program = GLFunctions::getShader( "mesh" );

    program->bind();

    GLFunctions::setupTextures();
    GLFunctions::setTextureUniforms( GLFunctions::getShader( "mesh" ), "maingl" );
    // Set modelview-projection matrix
    program->setUniformValue( "mvp_matrix", p_matrix * mv_matrix );
    program->setUniformValue( "mv_matrixInvert", mv_matrix.inverted() );

    QMatrix4x4 mat;
    program->setUniformValue( "userTransformMatrix", mat );

    program->setUniformValue( "u_colorMode", 2 );
    program->setUniformValue( "u_colormap", m_colormap );
    program->setUniformValue( "u_color", m_color.redF(), m_color.greenF(), m_color.blueF(), 1.0 );
    program->setUniformValue( "u_selectedMin", m_selectedMin );
    program->setUniformValue( "u_selectedMax", m_selectedMax );
    program->setUniformValue( "u_lowerThreshold", m_lowerThreshold );
    program->setUniformValue( "u_upperThreshold", m_upperThreshold );

    float nx = props.get( Fn::Property::D_NX ).toFloat();
    float ny = props.get( Fn::Property::D_NY ).toFloat();
    float nz = props.get( Fn::Property::D_NZ ).toFloat();
    float sx = props.get( Fn::Property::G_SAGITTAL ).toFloat();
    float sy = props.get( Fn::Property::G_CORONAL ).toFloat();
    float sz = props.get( Fn::Property::G_AXIAL ).toFloat();
    float dx = props.get( Fn::Property::D_DX ).toFloat();
    float dy = props.get( Fn::Property::D_DY ).toFloat();
    float dz = props.get( Fn::Property::D_DZ ).toFloat();

    program->setUniformValue( "u_x", sx * dx + dx / 2.0f );
    program->setUniformValue( "u_y", sy * dy + dy / 2.0f );
    program->setUniformValue( "u_z", sz * dz + dz / 2.0f );
    program->setUniformValue( "u_cutLowerX", false );
    program->setUniformValue( "u_cutLowerY", false );
    program->setUniformValue( "u_cutLowerZ", false );
    program->setUniformValue( "u_cutHigherX", false );
    program->setUniformValue( "u_cutHigherY", false );
    program->setUniformValue( "u_cutHigherZ", false );

    program->setUniformValue( "u_adjustX",   0.0f );
    program->setUniformValue( "u_adjustY", 0.0f );
    program->setUniformValue( "u_adjustZ", 0.0f );

    program->setUniformValue( "u_alpha", alpha );
    program->setUniformValue( "u_renderMode", renderMode );
    program->setUniformValue( "u_canvasSize", width, height );
    program->setUniformValue( "D0", 9 );
    program->setUniformValue( "D1", 10 );
    program->setUniformValue( "D2", 11 );
    program->setUniformValue( "P0", 12 );

    program->setUniformValue( "u_lighting", props.get( Fn::Property::D_LIGHT_SWITCH ).toBool() );
    program->setUniformValue( "u_lightAmbient", props.get( Fn::Property::D_LIGHT_AMBIENT ).toFloat() );
    program->setUniformValue( "u_lightDiffuse", props.get( Fn::Property::D_LIGHT_DIFFUSE ).toFloat() );
    program->setUniformValue( "u_materialAmbient", props.get( Fn::Property::D_MATERIAL_AMBIENT ).toFloat() );
    program->setUniformValue( "u_materialDiffuse", props.get( Fn::Property::D_MATERIAL_DIFFUSE ).toFloat() );
    program->setUniformValue( "u_materialSpecular", props.get( Fn::Property::D_MATERIAL_SPECULAR ).toFloat() );
    program->setUniformValue( "u_materialShininess", props.get( Fn::Property::D_MATERIAL_SHININESS ).toFloat() );


    float pAlpha =  1.0;
    float blue = (float) ( ( m_pickId ) & 0xFF ) / 255.f;
    float green = (float) ( ( m_pickId >> 8 ) & 0xFF ) / 255.f;
    float red = (float) ( ( m_pickId >> 16 ) & 0xFF ) / 255.f;
    program->setUniformValue( "u_pickColor", red, green , blue, pAlpha );

    program->setUniformValue( "u_dims", nx * dx, ny * dy, nz * dz );

    if ( !m_updateRunning )
    {
        initGeometry( props );
    }

    if ( m_glyphsUpdated )
    {
        updateGlyphs();
    }

    if ( m_tris == 0 )
    {
        return;
    }

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vboIds[ 0 ] );
    glBindBuffer( GL_ARRAY_BUFFER, vboIds[ 1 ] );
    setShaderVars( props );

    //glEnable(GL_CULL_FACE);
    //glCullFace( GL_BACK );
    glFrontFace( GL_CW );

    glDrawElements( GL_TRIANGLES, m_tris, GL_UNSIGNED_INT, 0 );

    glDisable(GL_CULL_FACE);

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void SHRenderer::setShaderVars( PropertyGroup& props )
{
    QGLShaderProgram* program = GLFunctions::getShader( "mesh" );

    program->bind();

    intptr_t offset = 0;
    // Tell OpenGL programmable pipeline how to locate vertex position data

    glBindBuffer( GL_ARRAY_BUFFER, vboIds[ 1 ] );
    int vertexLocation = program->attributeLocation( "a_position" );
    program->enableAttributeArray( vertexLocation );
    glVertexAttribPointer( vertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (const void *) offset );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glBindBuffer( GL_ARRAY_BUFFER, vboIds[ 3 ] );
    int normalLocation = program->attributeLocation( "a_normal" );
    program->enableAttributeArray( normalLocation );
    glVertexAttribPointer( normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (const void *) offset );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glBindBuffer( GL_ARRAY_BUFFER, vboIds[ 2 ] );
    int colorLocation = program->attributeLocation( "a_color" );
    program->enableAttributeArray( colorLocation );
    glVertexAttribPointer( colorLocation, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void SHRenderer::setRenderParams( PropertyGroup& props )
{
    m_scaling = props.get( Fn::Property::D_SCALING ).toFloat();
    m_offset = props.get( Fn::Property::D_OFFSET ).toInt();
    m_lod = props.get( Fn::Property::D_LOD ).toInt();
    m_minMaxScaling = props.get( Fn::Property::D_MINMAX_SCALING ).toBool();
    m_hideNegativeLobes = props.get( Fn::Property::D_HIDE_NEGATIVE_LOBES ).toBool();
    m_order = props.get( Fn::Property::D_ORDER ).toInt();

    m_orient = 0;
    if ( props.get( Fn::Property::D_RENDER_AXIAL ).toBool() )
    {
        m_orient = 1;
    }
    if ( props.get( Fn::Property::D_RENDER_CORONAL ).toBool() )
    {
        m_orient += 2;
    }
    if ( props.get( Fn::Property::D_RENDER_SAGITTAL ).toBool() )
    {
        m_orient += 4;
    }

    m_colorMode = props.get( Fn::Property::D_COLORMODE ).toInt();
    m_colormap = props.get( Fn::Property::D_COLORMAP ).toInt();
    m_selectedMin = props.get( Fn::Property::D_SELECTED_MIN ).toFloat();
    m_selectedMax = props.get( Fn::Property::D_SELECTED_MAX ).toFloat();
    m_lowerThreshold = props.get( Fn::Property::D_LOWER_THRESHOLD ).toFloat();
    m_upperThreshold = props.get( Fn::Property::D_UPPER_THRESHOLD ).toFloat();
    m_color = props.get( Fn::Property::D_COLOR ).value<QColor>();
}

void SHRenderer::initGeometry( PropertyGroup& props )
{
    float x = Models::getGlobal( Fn::Property::G_SAGITTAL ).toFloat();
    float y = Models::getGlobal( Fn::Property::G_CORONAL ).toFloat();
    float z = Models::getGlobal( Fn::Property::G_AXIAL ).toFloat();

    float zoom = Models::getGlobal( Fn::Property::G_ZOOM ).toFloat();
    float moveX = Models::getGlobal( Fn::Property::G_MOVEX ).toFloat();
    float moveY = Models::getGlobal( Fn::Property::G_MOVEY ).toFloat();

    QString s = createSettingsString( { x, y, z, m_orient, zoom, m_minMaxScaling, m_scaling, m_hideNegativeLobes, moveX, moveY, m_lod, m_offset } );
    if ( ( s == m_previousSettings ) || ( m_orient == 0 ) )
    {
        return;
    }
    m_previousSettings = s;

    m_updateRunning = true;
    m_glyphsUpdated = false;

    if ( m_updateThread )
    {
        delete m_updateThread;
    }
    m_updateThread = new SHRendererThread( m_data, m_pMatrix, m_mvMatrix, props );
    connect( m_updateThread, SIGNAL( finished( bool ) ), this, SLOT( updateThreadFinished( bool ) ) );
    m_updateThread->start();
}

void SHRenderer::updateThreadFinished( bool success )
{
    if ( success )
    {
        m_glyphsUpdated = true;
    }
    m_updateRunning = false;
    Models::g()->submit();
}

void SHRenderer::updateGlyphs()
{
    m_tris = m_updateThread->getIndices()->size();

    if ( m_tris == 0 )
    {
        return;
    }

    m_vertSize = m_updateThread->getVerts()->size();
    m_colorSize = m_updateThread->getColors()->size();

    glDeleteBuffers( 4, vboIds );
    glGenBuffers( 4, vboIds );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vboIds[ 0 ] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_tris * sizeof(GLuint), m_updateThread->getIndices()->data(), GL_STATIC_DRAW );

    glBindBuffer( GL_ARRAY_BUFFER, vboIds[ 1 ] );
    glBufferData( GL_ARRAY_BUFFER, m_updateThread->getVerts()->size() * sizeof(GLfloat), m_updateThread->getVerts()->data(), GL_STATIC_DRAW );

    glBindBuffer( GL_ARRAY_BUFFER, vboIds[ 2 ] );
    glBufferData( GL_ARRAY_BUFFER, m_updateThread->getColors()->size() * sizeof(GLfloat), m_updateThread->getColors()->data(), GL_DYNAMIC_DRAW );

    glBindBuffer( GL_ARRAY_BUFFER, vboIds[ 3 ] );
    glBufferData( GL_ARRAY_BUFFER, m_updateThread->getNormals()->size() * sizeof(GLfloat), m_updateThread->getNormals()->data(), GL_STATIC_DRAW );


    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    delete m_updateThread;
    m_updateThread = 0;
    m_glyphsUpdated = false;
}

TriangleMesh2* SHRenderer::getMesh()
{
    TriangleMesh2* mesh = new TriangleMesh2( m_vertSize / 3, m_tris / 3 );

    qDebug() << m_vertSize / 3 << m_tris / 3;

    float* verts = static_cast<float*>( glMapBuffer( vboIds[1], GL_READ_ONLY ) );

    for ( auto i = 0; i < m_vertSize / 3; ++i )
    {
        mesh->addVertex( verts[i*3], verts[i*3+1], verts[i*3+2] );
    }

    glUnmapBuffer( vboIds[1] );

    unsigned int* ids = static_cast<unsigned int*>( glMapBuffer( vboIds[0], GL_READ_ONLY ) );

    for ( auto i = 0; i < m_tris / 3; ++i )
    {
        mesh->addVertex( ids[i*3], ids[i*3+1], ids[i*3+2] );
    }

    glUnmapBuffer( vboIds[0] );

    return mesh;
}

