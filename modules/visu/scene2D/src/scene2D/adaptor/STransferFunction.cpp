/************************************************************************
 *
 * Copyright (C) 2009-2020 IRCAD France
 * Copyright (C) 2012-2020 IHU Strasbourg
 *
 * This file is part of Sight.
 *
 * Sight is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Sight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Sight. If not, see <https://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#include "scene2D/adaptor/STransferFunction.hpp"

#include <fwCom/Signal.hxx>

#include <fwData/mt/ObjectReadLock.hpp>
#include <fwData/mt/ObjectWriteLock.hpp>

#include <fwRenderQt/data/InitQtPen.hpp>
#include <fwRenderQt/data/Viewport.hpp>
#include <fwRenderQt/Scene2DGraphicsView.hpp>

#include <fwServices/macros.hpp>

#include <QColorDialog>
#include <QGraphicsItemGroup>
#include <QMenu>
#include <QPoint>

fwServicesRegisterMacro( ::fwRenderQt::IAdaptor, ::scene2D::adaptor::STransferFunction)

namespace scene2D
{
namespace adaptor
{

static const ::fwServices::IService::KeyType s_TF_INOUT       = "tf";
static const ::fwServices::IService::KeyType s_VIEWPORT_INPUT = "viewport";

static const std::string s_POLYGON_COLOR_CONFIG = "lineColor";
static const std::string s_POINT_COLOR_CONFIG   = "pointColor";
static const std::string s_POINT_SIZE_CONFIG    = "pointSize";
static const std::string s_INTERACTIVE_CONFIG   = "interactive";

//-----------------------------------------------------------------------------

STransferFunction::STransferFunction() noexcept
{
}

//-----------------------------------------------------------------------------

STransferFunction::~STransferFunction() noexcept
{
}

//-----------------------------------------------------------------------------

void STransferFunction::configuring()
{
    this->configureParams();

    const ConfigType tree = this->getConfigTree();
    const auto config     = tree.get_child("config.<xmlattr>");

    const std::string polygonColor = config.get(s_POLYGON_COLOR_CONFIG, "lightGray");
    ::fwRenderQt::data::InitQtPen::setPenColor(m_polygonsPen, polygonColor);

    const std::string pointColor = config.get(s_POINT_COLOR_CONFIG, "lightGray");
    ::fwRenderQt::data::InitQtPen::setPenColor(m_pointsPen, pointColor);

    m_pointSize   = config.get< float >(s_POINT_SIZE_CONFIG, m_pointSize);
    m_interactive = config.get< bool >(s_INTERACTIVE_CONFIG, m_interactive);
}

//------------------------------------------------------------------------------

void STransferFunction::starting()
{
    m_layer = new QGraphicsItemGroup();

    m_pointsPen.setCosmetic(true);
    m_pointsPen.setWidthF(0);

    m_polygonsPen.setCosmetic(true);
    m_polygonsPen.setWidthF(0);

    // Creates all entities.
    this->updating();
}

//------------------------------------------------------------------------------

::fwServices::IService::KeyConnectionsMap STransferFunction::getAutoConnections() const
{
    KeyConnectionsMap connections;
    connections.push(s_VIEWPORT_INPUT, ::fwRenderQt::data::Viewport::s_MODIFIED_SIG, s_UPDATE_SLOT);
    connections.push(s_TF_INOUT, ::fwData::TransferFunction::s_MODIFIED_SIG, s_UPDATE_SLOT);
    connections.push(s_TF_INOUT, ::fwData::TransferFunction::s_POINTS_MODIFIED_SIG, s_UPDATE_SLOT);
    connections.push(s_TF_INOUT, ::fwData::TransferFunction::s_WINDOWING_MODIFIED_SIG, s_UPDATE_SLOT);
    return connections;
}

//-----------------------------------------------------------------------------

void STransferFunction::updating()
{
    // Clears old data.
    this->destroyTFPolygon();
    this->destroyTFPoints();

    // Creates all TF.
    this->createTFPoints();
    this->createTFPolygon();

    // Builds the layer.
    this->buildLayer();
}

//-----------------------------------------------------------------------------

void STransferFunction::stopping()
{
    this->destroyTFPolygon();
    this->destroyTFPoints();
}

//------------------------------------------------------------------------------

void STransferFunction::createTFPoints()
{
    ::fwRenderQt::data::Viewport::sptr viewport = this->getScene2DRender()->getViewport();
    const ::fwData::mt::ObjectReadLock viewportLock(viewport);

    const double sceneWidth  = this->getScene2DRender()->getView()->width();
    const double sceneHeight = this->getScene2DRender()->getView()->height();

    // Computes point size in screen space and keep the smallest size (relativly to width or height).
    double pointSize = sceneWidth * m_pointSize;
    if(pointSize > sceneHeight * m_pointSize)
    {
        pointSize = sceneHeight * m_pointSize;
    }

    const double viewportWidth  = viewport->getWidth();
    const double viewportHeight = viewport->getHeight();

    // Computes point size from screen space to viewport space.
    const double pointWidth  = (viewportWidth * pointSize)/sceneWidth;
    const double pointHeight = (viewportHeight * pointSize)/sceneHeight;

    // Get the TF.
    const ::fwData::TransferFunction::csptr tf = this->getInOut< ::fwData::TransferFunction>(s_TF_INOUT);
    SLM_ASSERT("inout '" + s_TF_INOUT + "' does not exist.", tf);
    const ::fwData::mt::ObjectReadLock tfLock(tf);

    // Gets window/level informations to change TF value from TF space to window/level space.
    const ::fwData::TransferFunction::TFValuePairType minMaxValues = tf->getMinMaxTFValues();
    const ::fwData::TransferFunction::TFValueType minWL            = tf->getWLMinMax().first;
    const ::fwData::TransferFunction::TFValueType window           = tf->getWindow();
    const ::fwData::TransferFunction::TFValueType width            = minMaxValues.second - minMaxValues.first;

    // Fills TF point with color points.
    for(const ::fwData::TransferFunction::TFDataType::value_type& elt : tf->getTFData())
    {
        // Computes TF value from TF space to window/level space.
        ::fwData::TransferFunction::TFValueType value;
        value = (elt.first - minMaxValues.first) / width;
        value = value * window + minWL;

        // Creates the color.
        const ::fwData::TransferFunction::TFColor tfColor = elt.second;
        const Point2DType valColor(value, tfColor.a);
        Point2DType coord = this->mapAdaptorToScene(valColor, m_xAxis, m_yAxis);

        // Builds a point item, set its color, pen and zIndex.
        QGraphicsEllipseItem* point = new QGraphicsEllipseItem(
            coord.first - pointWidth / 2,
            coord.second - pointHeight / 2,
            pointWidth,
            pointHeight
            );
        QColor color(static_cast< int >(tfColor.r*255),
                     static_cast< int >(tfColor.g*255),
                     static_cast< int >(tfColor.b*255));
        point->setBrush(QBrush(color));
        point->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        point->setPen(m_pointsPen);
        point->setZValue(1);

        // Pushs it back into the point vector
        if(window > 0)
        {
            m_TFPoints.push_back(std::make_pair(coord, point));
        }
        // If the window is negative, the TF is reversed and point must be sort in reverse.
        else
        {
            m_TFPoints.insert(m_TFPoints.begin(), std::make_pair(coord, point));
        }
    }
}

//-----------------------------------------------------------------------------

void STransferFunction::destroyTFPoints()
{
    // Removes TF point items from the scene and clear the TF point vector.
    for(std::pair< Point2DType, QGraphicsEllipseItem* >& tfPoint : m_TFPoints)
    {
        this->getScene2DRender()->getScene()->removeItem(tfPoint.second);
        delete tfPoint.second;
    }
    m_TFPoints.clear();
}

//-----------------------------------------------------------------------------

void STransferFunction::createTFPolygon()
{
    const ::fwRenderQt::data::Viewport::sptr viewport = this->getScene2DRender()->getViewport();
    const ::fwData::mt::ObjectReadLock viewportLock(viewport);

    QVector<QPointF> position;
    QLinearGradient grad;

    const std::pair< Point2DType, QGraphicsEllipseItem* >& firstTFPoint = m_TFPoints.front();
    const std::pair< Point2DType, QGraphicsEllipseItem* >& lastTFPoint  = m_TFPoints.back();

    const QGraphicsEllipseItem* const firtsPoint = firstTFPoint.second;

    double xBegin = firstTFPoint.first.first;
    double xEnd   = lastTFPoint.first.first;

    // Get the TF.
    const ::fwData::TransferFunction::csptr tf = this->getInOut< ::fwData::TransferFunction>(s_TF_INOUT);
    SLM_ASSERT("inout '" + s_TF_INOUT + "' does not exist.", tf);
    const ::fwData::mt::ObjectReadLock tfLock(tf);

    if(tf->getIsClamped())
    {
        position.append(QPointF(xBegin, 0));
    }
    else
    {
        if(xBegin > viewport->getX())
        {
            xBegin = viewport->getX()-10;
            position.append(QPointF(xBegin, 0));
            position.append(QPointF(xBegin, firstTFPoint.first.second));
        }
        else
        {
            position.append(QPointF(xBegin, 0));
        }
        if(xEnd < viewport->getX() + viewport->getWidth())
        {
            xEnd = viewport->getX() + viewport->getWidth() + 10;
        }
    }

    grad.setColorAt(0, firtsPoint->brush().color());

    grad.setStart( xBegin, 0);
    grad.setFinalStop( xEnd, 0 );

    double distanceMax = xEnd - xBegin;

    // Iterates on TF points vector to add line and polygon items to the polygons vector.
    if(tf->getInterpolationMode() == ::fwData::TransferFunction::LINEAR)
    {
        this->buildLinearPolygons(position, grad, distanceMax);
    }
    else
    {
        this->buildNearestPolygons(position, grad, distanceMax);
    }

    if(!tf->getIsClamped())
    {
        if(xEnd == viewport->getX() + viewport->getWidth() + 10)
        {
            position.append(QPointF(xEnd, lastTFPoint.first.second));
        }
        const double lastPointX = lastTFPoint.first.first;
        grad.setColorAt((lastPointX-xBegin)/distanceMax, lastTFPoint.second->brush().color());
    }

    position.append(QPointF(xEnd, 0));
    grad.setColorAt(1, lastTFPoint.second->brush().color());

    QGraphicsPolygonItem* const poly = new QGraphicsPolygonItem(QPolygonF(position));
    // Sets gradient, opacity and pen to the polygon
    poly->setOpacity(m_opacity);
    poly->setPen(m_polygonsPen);
    poly->setCacheMode( QGraphicsItem::DeviceCoordinateCache );
    poly->setZValue(0);
    poly->setBrush(QBrush(grad));

    // Pushs the polygon back into the polygons vector
    m_TFPolygon = poly;
}

//-----------------------------------------------------------------------------

void STransferFunction::destroyTFPolygon()
{
    // Removes polygon items from the scene.
    if(m_TFPolygon)
    {
        this->getScene2DRender()->getScene()->removeItem(m_TFPolygon);
        delete m_TFPolygon;
    }
}

//-----------------------------------------------------------------------------

void STransferFunction::buildLinearPolygons(QVector<QPointF>& _position,
                                            QLinearGradient& _grad,
                                            double _distanceMax)
{
    for(auto tfPointIt = m_TFPoints.cbegin(); tfPointIt != m_TFPoints.cend()-1; ++tfPointIt)
    {
        const QPointF p1(tfPointIt->first.first, tfPointIt->first.second);
        const QPointF p2((tfPointIt+1)->first.first, (tfPointIt+1)->first.second);

        _position.append(p1);
        _position.append(p2);

        // Builds the gradient
        _grad.setColorAt((p1.x() - _position[0].x())/_distanceMax, (tfPointIt->second)->brush().color());
    }
}

//-----------------------------------------------------------------------------

void STransferFunction::buildNearestPolygons(QVector<QPointF>& _position,
                                             QLinearGradient& _grad,
                                             double _distanceMax)
{
    for(auto tfPointIt = m_TFPoints.cbegin(); tfPointIt != m_TFPoints.cend()-1; ++tfPointIt)
    {
        const QPointF p1(tfPointIt->first.first, tfPointIt->first.second);
        const QPointF p4((tfPointIt+1)->first.first, (tfPointIt+1)->first.second);

        const QPointF p2(p1.x() + (p4.x() - p1.x())/2., p1.y());
        const QPointF p3(p2.x(), p4.y());

        _position.append(p1);
        _position.append(p2);
        _position.append(p3);
        _position.append(p4);

        const double d1 = (p1.x() - _position[0].x())/_distanceMax;
        const double d2 = (p2.x() - _position[0].x())/_distanceMax;
        const double d3 = d2 + std::numeric_limits< double >::epsilon();
        const double d4 = (p4.x() - _position[0].x())/_distanceMax;

        const QColor c1 = (tfPointIt->second)->brush().color();
        const QColor c4 = ((tfPointIt+1)->second)->brush().color();

        _grad.setColorAt(d1, c1);
        _grad.setColorAt(d2, c1);
        _grad.setColorAt(d3, c4);
        _grad.setColorAt(d4, c4);
    }
}

//-----------------------------------------------------------------------------

void STransferFunction::buildLayer()
{
    // Adds graphics items vectors to the layer.
    for(std::pair< Point2DType, QGraphicsEllipseItem* >& tfPoint : m_TFPoints)
    {
        m_layer->addToGroup(tfPoint.second);
    }
    m_layer->addToGroup(m_TFPolygon);

    // Adjusts the layer's position and zValue depending on the associated axis.
    m_layer->setPos(m_xAxis->getOrigin(), m_yAxis->getOrigin());
    m_layer->setZValue(m_zValue);

    // Adds the layer item to the scene.
    this->getScene2DRender()->getScene()->addItem(m_layer);
}

//------------------------------------------------------------------------------

void STransferFunction::processInteraction(::fwRenderQt::data::Event& _event)
{
    if(!m_interactive)
    {
        return;
    }

    // If it's a resize event, all the scene must be recompted.
    if(_event.getType() == ::fwRenderQt::data::Event::Resize)
    {
        this->updating();
        _event.setAccepted(true);
        return;
    }

    // If a point as already been captured.
    if(m_capturedTFPoint)
    {
        if(_event.getButton() == ::fwRenderQt::data::Event::LeftButton &&
           _event.getType() == ::fwRenderQt::data::Event::MouseButtonRelease)
        {
            // Releases capture point.
            this->leftButtonReleaseEvent();
            _event.setAccepted(true);
            return;
        }
    }

    // If a TF as already been captured.
    if(m_capturedTF.first)
    {
        if(_event.getType() == ::fwRenderQt::data::Event::MouseMove)
        {
            // Changes the subTF level.
            this->mouseMoveOnTFEvent(_event);
            _event.setAccepted(true);
            return;
        }
        else if(_event.getButton() == ::fwRenderQt::data::Event::MidButton &&
                _event.getType() == ::fwRenderQt::data::Event::MouseButtonRelease)
        {
            // Releases capture subTF.
            this->midButtonReleaseEvent();
            _event.setAccepted(true);
            return;
        }
    }

    const QPoint scenePos = QPoint(static_cast< int >(_event.getCoord().getX()),
                                   static_cast< int >(_event.getCoord().getY()));
    QList<QGraphicsItem*> items = this->getScene2DRender()->getView()->items(scenePos);

    // Checks if a point is clicked.
    for(std::pair< Point2DType, QGraphicsEllipseItem* >& tfPoint : m_TFPoints)
    {
        // If a point as already been captured.
        if(m_capturedTFPoint == &tfPoint)
        {
            if(_event.getType() == ::fwRenderQt::data::Event::MouseMove)
            {
                // Moves the captured point.
                this->mouseMoveOnPointEvent(_event);
                _event.setAccepted(true);
                return;
            }

        }
        else if(items.indexOf(tfPoint.second) >= 0)
        {
            // If there is a double click on a point, open a color dialog.
            if(_event.getButton() == ::fwRenderQt::data::Event::LeftButton &&
               _event.getType() == ::fwRenderQt::data::Event::MouseButtonDoubleClick)
            {
                this->leftButtonDoubleClickOnPointEvent(tfPoint);
                _event.setAccepted(true);
                return;
            }
            // If left button is pressed on a point, set the TF as current.
            else if(_event.getButton() == ::fwRenderQt::data::Event::LeftButton &&
                    _event.getType() == ::fwRenderQt::data::Event::MouseButtonPress)
            {
                this->leftButtonClickOnPointEvent(tfPoint);
                _event.setAccepted(true);
                return;
            }
            // If right button is pressed on a point, remove it.
            if(_event.getButton() == ::fwRenderQt::data::Event::RightButton &&
               _event.getType() == ::fwRenderQt::data::Event::MouseButtonPress)
            {
                this->rightButtonClickOnPointEvent(tfPoint);
                _event.setAccepted(true);
                return;
            }
        }
    }

    // Adds a new TF point.
    if(_event.getButton() == ::fwRenderQt::data::Event::LeftButton &&
       _event.getType() == ::fwRenderQt::data::Event::MouseButtonDoubleClick)
    {
        this->leftButtonDoubleClickEvent(_event);
        _event.setAccepted(true);
        return;
    }

    // If midlle button is pressed, select the current TF to adjust the window/level.
    if(_event.getButton() == ::fwRenderQt::data::Event::MidButton &&
       _event.getType() == ::fwRenderQt::data::Event::MouseButtonPress)
    {
        this->midButtonClickEvent(_event);
        _event.setAccepted(true);
        return;
    }

    // If right button is pressed, open a context menu to manage multiple actions.
    if(_event.getButton() == ::fwRenderQt::data::Event::RightButton &&
       _event.getType() == ::fwRenderQt::data::Event::MouseButtonPress)
    {
        this->rightButtonCLickEvent(_event);
        _event.setAccepted(true);
        return;
    }
}

//-----------------------------------------------------------------------------

void STransferFunction::leftButtonClickOnPointEvent(std::pair< Point2DType, QGraphicsEllipseItem* >& _TFPoint)
{
    // Stores the captured TF point in case it's moved.
    m_capturedTFPoint = &_TFPoint;

    // Sets the selected point pen to lighter to get a visual feedback that the selected point is selected.
    const QColor color = _TFPoint.second->brush().color();
    QPen tfPointPen(color);
    tfPointPen.setCosmetic(true);
    _TFPoint.second->setPen(tfPointPen);
}

//-----------------------------------------------------------------------------

void STransferFunction::mouseMoveOnPointEvent(const ::fwRenderQt::data::Event& _event)
{
    // m_capturedTFPoint must be previously sets by
    // leftButtonClickOnPointEvent(std::pair< Point2DType, QGraphicsEllipseItem* >&)
    SLM_ASSERT("The captured TF point must exist", m_capturedTFPoint);

    const auto pointIt = std::find(m_TFPoints.begin(), m_TFPoints.end(), *m_capturedTFPoint);
    SLM_ASSERT("The captured point is not found", pointIt != m_TFPoints.end());

    // Gets the previous point of the TF.
    auto previousPoint = pointIt;
    if(*m_capturedTFPoint != m_TFPoints.front())
    {
        --previousPoint;
    }

    // Gets the next point of the TF.
    auto nextPoint = pointIt;
    if(*m_capturedTFPoint != m_TFPoints.back())
    {
        ++nextPoint;
    }

    // Gets position informations of the previous and the next point.
    const double previousPointXCoord = previousPoint->first.first;
    const double nextPointXCoord     = nextPoint->first.first;

    // Gets the actual mouse point coordinates.
    ::fwRenderQt::data::Coord newCoord = this->getScene2DRender()->mapToScene(_event.getCoord());

    // Clamps new y coord between -1 and 0.
    if(newCoord.getY() > 0)
    {
        newCoord.setY(0);
    }
    if(newCoord.getY() < -1)
    {
        newCoord.setY(-1);
    }

    // Clamps new x coord between the previous and the next one.
    const double delta = 1.;
    if(*m_capturedTFPoint == m_TFPoints.front())
    {
        if(newCoord.getX() > nextPointXCoord)
        {
            newCoord.setX(nextPointXCoord - delta);
        }
    }
    else if(*m_capturedTFPoint == m_TFPoints.back())
    {
        if(newCoord.getX() < previousPointXCoord)
        {
            newCoord.setX(previousPointXCoord + delta);
        }
    }
    else
    {
        if(newCoord.getX() < previousPointXCoord)
        {
            newCoord.setX(previousPointXCoord + delta);
        }
        else if(newCoord.getX() > nextPointXCoord)
        {
            newCoord.setX(nextPointXCoord - delta);
        }
    }

    // Moves the selected TF point by the difference between the old coordinates and the new ones.
    m_capturedTFPoint->second->moveBy(newCoord.getX() - m_capturedTFPoint->first.first,
                                      newCoord.getY() - m_capturedTFPoint->first.second);

    // Stores new coordinates to the captured one.
    m_capturedTFPoint->first.first  = newCoord.getX();
    m_capturedTFPoint->first.second = newCoord.getY();

    // Re-draw the current polygons.
    this->destroyTFPolygon();
    this->createTFPolygon();
    this->buildLayer();

    // Updates the TF with the new point position.
    size_t pointIndex = pointIt - m_TFPoints.begin();

    // Get the TF.
    const ::fwData::TransferFunction::sptr tf = this->getInOut< ::fwData::TransferFunction>(s_TF_INOUT);
    SLM_ASSERT("inout '" + s_TF_INOUT + "' does not exist.", tf);
    const ::fwData::mt::ObjectWriteLock tfLock(tf);

    // If the window is negative, the TF point list is reversed compared to the TF data.
    if(tf->getWindow() < 0)
    {
        pointIndex = m_TFPoints.size() - 1 - pointIndex;
    }
    // Retrieves the TF point.
    ::fwData::TransferFunction::TFDataType tfData = tf->getTFData();
    auto tfDataIt = tfData.begin();
    for(unsigned i = 0; i < pointIndex; ++i)
    {
        tfDataIt++;
    }

    // Gets the TF point information
    ::fwData::TransferFunction::TFValueType oldTFValue = tfDataIt->first;
    ::fwData::TransferFunction::TFColor tfColor        = tfDataIt->second;

    // Gets window/level informations to change TF value from TF space to window/level space.
    const ::fwData::TransferFunction::TFValuePairType minMaxValues = tf->getMinMaxTFValues();
    const ::fwData::TransferFunction::TFValueType minWL            = tf->getWLMinMax().first;
    const ::fwData::TransferFunction::TFValueType window           = tf->getWindow();
    const ::fwData::TransferFunction::TFValueType width            = minMaxValues.second - minMaxValues.first;

    // Gets new window/level min max value in the window/level space.
    const double min = m_TFPoints.begin()->first.first;
    const double max = m_TFPoints.rbegin()->first.first;

    // Removes the old TF point.
    tf->eraseTFValue(oldTFValue);

    // Updates the color alpha channel.
    tfColor.a = std::abs(newCoord.getY());

    // Computes TF value from window/level space to TF space.
    ::fwData::TransferFunction::TFValueType newTFValue = newCoord.getX();
    newTFValue                                         = (newTFValue - minWL) / window;
    newTFValue                                         = (newTFValue * width) + minMaxValues.first;

    // Adds the new TF point.
    tf->addTFColor(newTFValue, tfColor);

    // Updates the window/level.
    if(window > 0)
    {
        tf->setWLMinMax(::fwData::TransferFunction::TFValuePairType(min, max));
    }
    else
    {
        tf->setWLMinMax(::fwData::TransferFunction::TFValuePairType(max, min));
    }

    // Sends the modification signal.
    const auto sig = tf->signal< ::fwData::TransferFunction::PointsModifiedSignalType >(
        ::fwData::TransferFunction::s_POINTS_MODIFIED_SIG);
    {
        const ::fwCom::Connection::Blocker block(sig->getConnection(m_slotUpdate));
        sig->asyncEmit();
    }
}

//-----------------------------------------------------------------------------

void STransferFunction::leftButtonReleaseEvent()
{
    // Removes the hightlighting of the captured point.
    m_capturedTFPoint->second->setPen(m_pointsPen);
    m_capturedTFPoint = nullptr;
}

//-----------------------------------------------------------------------------

void STransferFunction::leftButtonDoubleClickOnPointEvent(std::pair< Point2DType, QGraphicsEllipseItem* >& _TFPoint)
{
    // Opens a QColorDialog with the selected circle color and the tf point alpha as default rgba color.
    QColor oldColor = _TFPoint.second->brush().color();
    oldColor.setAlphaF(-_TFPoint.first.second);

    QColor newColor = QColorDialog::getColor(oldColor,
                                             this->getScene2DRender()->getView(),
                                             QString("Choose the point color"),
                                             QColorDialog::ShowAlphaChannel);

    if(newColor.isValid())
    {
        // Updates the TF.
        auto pointIt =
            std::find(m_TFPoints.begin(), m_TFPoints.end(), _TFPoint);
        SLM_ASSERT("The captured point is not found", pointIt != m_TFPoints.end());
        size_t pointIndex = pointIt - m_TFPoints.begin();

        // Get the TF.
        const ::fwData::TransferFunction::sptr tf = this->getInOut< ::fwData::TransferFunction>(s_TF_INOUT);
        SLM_ASSERT("inout '" + s_TF_INOUT + "' does not exist.", tf);

        {
            // If the window is negative, the TF point list is reversed compared to the TF data.
            const ::fwData::mt::ObjectReadLock tfLock(tf);
            if(tf->getWindow() < 0)
            {
                pointIndex = m_TFPoints.size() - 1 - pointIndex;
            }

            // Retrieves the TF point.
            ::fwData::TransferFunction::TFDataType tfData = tf->getTFData();
            auto tfDataIt = tfData.begin();
            for(unsigned i = 0; i < pointIndex; ++i)
            {
                tfDataIt++;
            }

            // Removes the TF point.
            ::fwData::TransferFunction::TFValueType tfValue = tfDataIt->first;

            // Removes the old TF point.
            tf->eraseTFValue(tfValue);

            // Adds the new one with the new color.
            ::fwData::TransferFunction::TFColor tfColor(newColor.red()/255.,
                                                        newColor.green()/255.,
                                                        newColor.blue()/255.,
                                                        newColor.alpha()/255.);
            tf->addTFColor(tfValue, tfColor);
        }

        // Sends the modification signal.
        const auto sig = tf->signal< ::fwData::TransferFunction::PointsModifiedSignalType >(
            ::fwData::TransferFunction::s_POINTS_MODIFIED_SIG);
        {
            const ::fwCom::Connection::Blocker block(sig->getConnection(m_slotUpdate));
            sig->asyncEmit();
        }

        // Updates the displayed TF point.
        const double newYPos = -newColor.alpha()/255.;
        _TFPoint.second->moveBy(0.,  oldColor.alphaF() + newYPos);
        _TFPoint.first.second = newYPos;
        newColor.setAlpha(255);
        _TFPoint.second->setBrush(QBrush(newColor));

        // Re-draw the current polygons.
        this->destroyTFPolygon();
        this->createTFPolygon();
        this->buildLayer();
    }
}

//-----------------------------------------------------------------------------

void STransferFunction::rightButtonClickOnPointEvent(std::pair< Point2DType, QGraphicsEllipseItem* >& _TFPoint)
{
    // Updates the TF.
    auto pointIt =
        std::find(m_TFPoints.begin(), m_TFPoints.end(), _TFPoint);
    SLM_ASSERT("The captured point is not found", pointIt != m_TFPoints.end());
    size_t pointIndex = pointIt - m_TFPoints.begin();

    // Get the TF.
    const ::fwData::TransferFunction::sptr tf = this->getInOut< ::fwData::TransferFunction>(s_TF_INOUT);
    SLM_ASSERT("inout '" + s_TF_INOUT + "' does not exist.", tf);

    {
        // If the window is negative, the TF point list is reversed compared to the TF data.
        const ::fwData::mt::ObjectReadLock tfLock(tf);
        const double window = tf->getWindow();
        if(window <= 0)
        {
            pointIndex = m_TFPoints.size() - 1 - pointIndex;
        }

        // Retrieves the TF point.
        ::fwData::TransferFunction::TFDataType tfData = tf->getTFData();
        auto tfDataIt = tfData.begin();
        for(unsigned i = 0; i < pointIndex; ++i)
        {
            tfDataIt++;
        }

        // Removes the TF point.
        const ::fwData::TransferFunction::TFValueType tfValue = tfDataIt->first;
        tf->eraseTFValue(tfValue);

        // Gets new window/level min max value in the window/level space.
        double min = m_TFPoints.begin()->first.first;
        double max = m_TFPoints.rbegin()->first.first;

        // If the removed point is the last or the first, the min max is wrong and need to be updated.
        if((pointIndex == 0 && window >= 0) || (pointIndex == m_TFPoints.size()-1 && window < 0))
        {
            min = (m_TFPoints.begin()+1)->first.first;
        }
        else if((pointIndex == m_TFPoints.size()-1 && window >= 0) || (pointIndex == 0 && window < 0))
        {
            max = (m_TFPoints.rbegin()+1)->first.first;
        }

        // Updates the window/level.
        if(window > 0)
        {
            tf->setWLMinMax(::fwData::TransferFunction::TFValuePairType(min, max));
        }
        else
        {
            tf->setWLMinMax(::fwData::TransferFunction::TFValuePairType(max, min));
        }
    }

    // Sends the modification signal.
    const auto sig = tf->signal< ::fwData::TransferFunction::PointsModifiedSignalType >(
        ::fwData::TransferFunction::s_POINTS_MODIFIED_SIG);
    {
        const ::fwCom::Connection::Blocker block(sig->getConnection(m_slotUpdate));
        sig->asyncEmit();
    }

    this->getScene2DRender()->getScene()->removeItem(pointIt->second);
    delete pointIt->second;
    m_TFPoints.erase(pointIt);

    // Re-draw the current polygons.
    this->destroyTFPolygon();
    this->createTFPolygon();
    this->buildLayer();
}

//-----------------------------------------------------------------------------

void STransferFunction::leftButtonDoubleClickEvent(const ::fwRenderQt::data::Event& _event)
{
    ::fwRenderQt::data::Coord newCoord = this->getScene2DRender()->mapToScene(_event.getCoord());

    // Clamps new y coord between -1 and 0.
    if(newCoord.getY() > 0)
    {
        newCoord.setY(0);
    }
    if(newCoord.getY() < -1)
    {
        newCoord.setY(-1);
    }

    // Get the TF.
    const ::fwData::TransferFunction::sptr tf = this->getInOut< ::fwData::TransferFunction>(s_TF_INOUT);
    SLM_ASSERT("inout '" + s_TF_INOUT + "' does not exist.", tf);
    {
        const ::fwData::mt::ObjectWriteLock tfLock(tf);

        ::fwData::TransferFunction::TFColor newColor;

        // The new coord becomes the new first TF point, get the current first color in the list.
        if(newCoord.getX() < m_TFPoints.front().first.first)
        {
            const QColor firstColor = m_TFPoints.front().second->brush().color();
            newColor = ::fwData::TransferFunction::TFColor(firstColor.red()/255.,
                                                           firstColor.green()/255.,
                                                           firstColor.blue()/255.,
                                                           -newCoord.getY());

        }
        // The new coord becomes the new last TF point, get the current last color in the list.
        else if(newCoord.getX() > m_TFPoints.back().first.first)
        {
            const QColor firstColor = m_TFPoints.back().second->brush().color();
            newColor = ::fwData::TransferFunction::TFColor(firstColor.red()/255.,
                                                           firstColor.green()/255.,
                                                           firstColor.blue()/255.,
                                                           -newCoord.getY());

        }
        // Gets an interpolate color since the new point is between two ohers.
        else
        {
            newColor   = tf->getInterpolatedColor(newCoord.getX());
            newColor.a = -newCoord.getY();
        }

        // Gets window/level informations to change TF value from TF space to window/level space.
        const ::fwData::TransferFunction::TFValuePairType minMaxValues = tf->getMinMaxTFValues();
        const ::fwData::TransferFunction::TFValueType minWL            = tf->getWLMinMax().first;
        const ::fwData::TransferFunction::TFValueType window           = tf->getWindow();
        const ::fwData::TransferFunction::TFValueType width            = minMaxValues.second - minMaxValues.first;

        // Computes TF value from window/level space to TF space.
        ::fwData::TransferFunction::TFValueType tfValue = newCoord.getX();
        tfValue                                         = (tfValue - minWL) / window;
        tfValue                                         = (tfValue * width) + minMaxValues.first;

        // Adds the new TF point.
        tf->addTFColor(tfValue, newColor);

        // Gets new window/level min max value in the window/level space.
        double min = m_TFPoints.begin()->first.first;
        double max = m_TFPoints.rbegin()->first.first;

        if(newCoord.getX() > max)
        {
            max = newCoord.getX();
        }
        else if(newCoord.getX() < min)
        {
            min = newCoord.getX();
        }

        // Updates the window/level.
        if(window > 0)
        {
            tf->setWLMinMax(::fwData::TransferFunction::TFValuePairType(min, max));
        }
        else
        {
            tf->setWLMinMax(::fwData::TransferFunction::TFValuePairType(max, min));
        }
    }

    // Sends the signal.
    const auto sig = tf->signal< ::fwData::TransferFunction::PointsModifiedSignalType >(
        ::fwData::TransferFunction::s_POINTS_MODIFIED_SIG);
    {
        const ::fwCom::Connection::Blocker block(sig->getConnection(m_slotUpdate));
        sig->asyncEmit();
    }

    // Re-draw all the scene.
    this->updating();
}

//-----------------------------------------------------------------------------

void STransferFunction::midButtonClickEvent(const ::fwRenderQt::data::Event& _event)
{
    const QPoint scenePos = QPoint(static_cast< int >(_event.getCoord().getX()),
                                   static_cast< int >(_event.getCoord().getY()));
    QList<QGraphicsItem*> items = this->getScene2DRender()->getView()->items(scenePos);

    // Checks if a polygon is clicked.
    if(items.indexOf(m_TFPolygon) >= 0)
    {
        this->getScene2DRender()->getView()->setCursor(Qt::ClosedHandCursor);
        ::fwRenderQt::data::Coord windowLevelCoord = this->getScene2DRender()->mapToScene(_event.getCoord());

        // Get the TF.
        const ::fwData::TransferFunction::sptr tf = this->getInOut< ::fwData::TransferFunction>(s_TF_INOUT);
        SLM_ASSERT("inout '" + s_TF_INOUT + "' does not exist.", tf);

        // Stores the level in window/level space and the window in screen space.
        m_capturedTF =
            std::make_pair(tf, ::fwRenderQt::data::Coord(windowLevelCoord.getX(), _event.getCoord().getY()));
    }
}

//-----------------------------------------------------------------------------

void STransferFunction::mouseMoveOnTFEvent(const ::fwRenderQt::data::Event& _event)
{
    // m_capturedTF must be previously sets by midButtonClickEvent(const ::fwRenderQt::data::Event& _event)
    SLM_ASSERT("The captured subTF must exist", m_capturedTF.first);

    const ::fwRenderQt::data::Coord windowLevelCoord = this->getScene2DRender()->mapToScene(_event.getCoord());

    // The level delta is in window/level space.
    const double levelDelta = windowLevelCoord.getX() - m_capturedTF.second.getX();

    // The window delta is in screen space.
    const double windowDelta = _event.getCoord().getY() - m_capturedTF.second.getY();

    // Updates the TF.
    const ::fwData::TransferFunction::sptr tf = m_capturedTF.first;
    {
        const ::fwData::mt::ObjectWriteLock tfLock(tf);
        tf->setWindow(tf->getWindow() - windowDelta);
        tf->setLevel(tf->getLevel() + levelDelta);

        // Sends the signal.
        const auto sig = tf->signal< ::fwData::TransferFunction::WindowingModifiedSignalType >(
            ::fwData::TransferFunction::s_WINDOWING_MODIFIED_SIG);
        {
            const ::fwCom::Connection::Blocker block(sig->getConnection(m_slotUpdate));
            sig->asyncEmit(tf->getWindow(), tf->getLevel());
        }
    }

    // Stores the level in window/level space and the window in screen space.
    m_capturedTF.second = ::fwRenderQt::data::Coord(windowLevelCoord.getX(), _event.getCoord().getY());

    // Re-draw all the scene.
    this->updating();
}

//-----------------------------------------------------------------------------

void STransferFunction::midButtonReleaseEvent()
{
    this->getScene2DRender()->getView()->setCursor(Qt::ArrowCursor);
    m_capturedTF.first = nullptr;
}

//-----------------------------------------------------------------------------

void STransferFunction::rightButtonCLickEvent(const ::fwRenderQt::data::Event& _event)
{
    const QPoint scenePos = QPoint(static_cast< int >(_event.getCoord().getX()),
                                   static_cast< int >(_event.getCoord().getY()));
    QList<QGraphicsItem*> items = this->getScene2DRender()->getView()->items(scenePos);

    // Checks if a polygon is clicked.
    if(items.indexOf(m_TFPolygon) >= 0)
    {
        // Get the TF.
        const ::fwData::TransferFunction::sptr tf = this->getInOut< ::fwData::TransferFunction>(s_TF_INOUT);
        SLM_ASSERT("inout '" + s_TF_INOUT + "' does not exist.", tf);

        // Creates the menu.
        QMenu* const contextMenu = new QMenu();

        // Adds the clamp action.
        QAction* const clampAction = new QAction("Clamp");
        clampAction->setCheckable(true);
        clampAction->setChecked(tf->getIsClamped());
        QObject::connect(clampAction, &QAction::triggered, this, &STransferFunction::clampTF);
        contextMenu->addAction(clampAction);

        // Adds the interpolation mode action.
        QAction* const linearAction = new QAction("Linear");
        linearAction->setCheckable(true);
        linearAction->setChecked(tf->getInterpolationMode() == ::fwData::TransferFunction::LINEAR);
        QObject::connect(linearAction, &QAction::triggered, this, &STransferFunction::toggleLinearTF);
        contextMenu->addAction(linearAction);

        // Opens the menu.
        contextMenu->exec(QCursor::pos());

        delete contextMenu;
    }
}

//-----------------------------------------------------------------------------

void STransferFunction::clampTF(bool _clamp)
{
    // Get the TF.
    const ::fwData::TransferFunction::sptr tf = this->getInOut< ::fwData::TransferFunction>(s_TF_INOUT);
    SLM_ASSERT("inout '" + s_TF_INOUT + "' does not exist.", tf);

    // Set the clamp status.
    {
        const ::fwData::mt::ObjectWriteLock tfLock(tf);
        tf->setIsClamped(_clamp);
    }

    // Sends the signal.
    const auto sig = tf->signal< ::fwData::TransferFunction::ModifiedSignalType >(
        ::fwData::TransferFunction::s_MODIFIED_SIG);
    {
        const ::fwCom::Connection::Blocker block(sig->getConnection(m_slotUpdate));
        sig->asyncEmit();
    }

    // Re-draw the current polygons.
    this->destroyTFPolygon();
    this->createTFPolygon();
    this->buildLayer();
}

//-----------------------------------------------------------------------------

void STransferFunction::toggleLinearTF(bool _linear)
{
    // Get the TF.
    const ::fwData::TransferFunction::sptr tf = this->getInOut< ::fwData::TransferFunction>(s_TF_INOUT);
    SLM_ASSERT("inout '" + s_TF_INOUT + "' does not exist.", tf);

    // Set the interpolation mode.
    {
        const ::fwData::mt::ObjectWriteLock tfLock(tf);
        tf->setInterpolationMode(_linear ? ::fwData::TransferFunction::LINEAR : ::fwData::TransferFunction::NEAREST);
    }

    // Sends the signal.
    const auto sig = tf->signal< ::fwData::TransferFunction::ModifiedSignalType >(
        ::fwData::TransferFunction::s_MODIFIED_SIG);
    {
        const ::fwCom::Connection::Blocker block(sig->getConnection(m_slotUpdate));
        sig->asyncEmit();
    }

    // Re-draw the current polygons.
    this->destroyTFPolygon();
    this->createTFPolygon();
    this->buildLayer();
}

} // namespace adaptor
} // namespace scene2D
