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

#pragma once

#include "scene2D/config.hpp"

#include <fwData/TransferFunction.hpp>

#include <fwDataTools/helper/MedicalImage.hpp>

#include <fwRenderQt/IAdaptor.hpp>

namespace scene2D
{
namespace adaptor
{

/**
 * @brief IAdaptor implementation to display a transfer function.
 *
 * The following actions are available:
 * - Left mouse click: move the current clicked TF point.
 * - Left mouse double click: adds a new TF point to the TF or open a color dialog
 *                            to change the current clicked TF point.
 * - Middle mouse click: adjusts the transfer function level and window by moving
 *                       the mouse up/down and left/right respectively.
 * - Right mouse click: remove the current clicked TF point or open a context menu
 *
 * @section XML XML Configuration
 *
 * @code{.xml}
   <service uid="tf2" type="::scene2D::adaptor::STransferFunction" autoConnect="yes">
       <inout key="tf" uid="..." />
       <inout key="viewport" uid="..." />
       <config lineColor="lightGray" pointColor="lightGray" xAxis="xAxis" yAxis="yAxis" zValue="0" />
   </service>
   @endcode
 *
 * @subsection In-Out In-Out
 * - \b tf [::fwData::TransferFunction]: the current TransferFunction.
 * - \b viewport [::fwRenderQt::data::Viewport]: object listened to update the adaptor.
 *
 * @subsection Configuration Configuration:
 * - \b config (mandatory): contains the adaptor configuration
 *    - \b xAxis (optional): x axis associated to the adaptor
 *    - \b yAxis (optional): y axis associated to the adaptor
 *    - \b zValue (optional, default=0): z value of the layer
 *    - \b lineColor (optional, default black): Set the color of the lines between the TF points.
 *    - \b pointColor (optional, default="lightGray"): outline color of the circles representing the TF points.
 *    - \b pointSize (optional, default=0.03): size of TF points in a ratio relative to the window.
 *    - \b opacity (optional, default=1.0): opacity of the gradient.
 *    - \b interactive (optional, true/false, default=false): enables interactions.
 */
class SCENE2D_CLASS_API STransferFunction :
    public QObject,
    public ::fwRenderQt::IAdaptor
{

Q_OBJECT

public:

    fwCoreServiceMacro(STransferFunction, ::fwRenderQt::IAdaptor)

    /// Creates the adaptor.
    SCENE2D_API STransferFunction() noexcept;

    /// Destroys the adaptor.
    SCENE2D_API virtual ~STransferFunction() noexcept;

private:

    /// Configures the adaptor.
    virtual void configuring() override;

    /**
     * @brief Initializes the layer and draw the TF.
     *
     * @see updating()
     */
    virtual void starting() override;

    /**
     * @brief Proposals to connect service slots to associated object signals.
     * @return A map of each proposed connection.
     *
     * Connect ::fwRenderQt::data::Viewport::s_MODIFIED_SIG of s_VIEWPORT_INPUT to
     * ::scene2D::adaptor::SMultipleTF::s_UPDATE_SLOT.
     * Connect ::fwData::TransferFunction::s_MODIFIED_SIG of s_TF_INOUT to
     * ::scene2D::adaptor::STransferFunction::s_UPDATE_SLOT.
     * Connect ::fwData::TransferFunction::s_POINTS_MODIFIED_SIG of s_TF_INOUT to
     * ::scene2D::adaptor::STransferFunction::s_UPDATE_SLOT.
     * Connect ::fwData::TransferFunction::s_WINDOWING_MODIFIED_SIG of s_TF_INOUT to
     * ::scene2D::adaptor::STransferFunction::s_UPDATE_SLOT.
     */
    virtual KeyConnectionsMap getAutoConnections() const override;

    /// Release all graphics items and draw the TF.
    virtual void updating() override;

    /// Release all graphic items.
    virtual void stopping() override;

    /// Creates graphic points.
    void createTFPoints();

    /// Removes all graphic points from the layer and deletes it.
    void destroyTFPoints();

    /// Creates the gradient.
    void createTFPolygon();

    /// Removes the graphic gradient from the layer and deletes it.
    void destroyTFPolygon();

    /**
     * @brief Creates lines and linear gradient polygons of the TF.
     * @param _position the position vector to fill.
     * @param _grad the gradient to create.
     * @param _distanceMax the maximum distance used by the gradient.
     */
    void buildLinearPolygons(QVector<QPointF>& _position,
                             QLinearGradient& _grad,
                             double _distanceMax);

    /**
     * @brief Creates lines and nearest gradient polygons of the TF.
     * @param _position the position vector to fill.
     * @param _grad the gradient to create.
     * @param _distanceMax the maximum distance used by the gradient.
     */
    void buildNearestPolygons(QVector<QPointF>& _position,
                              QLinearGradient& _grad,
                              double _distanceMax);

    /// Adds graphic items to @ref m_layer.
    void buildLayer();

    /**
     * @brief Filters the event to call the right methods from mouse informations.
     * @param _event the 2D scene event.
     *
     * The following actions are available:
     * - Left mouse click: move the current clicked TF point.
     * - Left mouse double click: adds a new TF point to the TF or open a color dialog
     *                            to change the current clicked TF point.
     * - Middle mouse click: adjusts the transfer function level and window by moving
     *                       the mouse up/down and left/right respectively.
     * - Right mouse click: remove the current clicked TF point or open a context menu
     *                      to manage multiple actions which are 'clamp' or 'linear'.
     */
    virtual void processInteraction(::fwRenderQt::data::Event& _event ) override;

    /**
     * @brief Sets @ref m_capturedTFPoint and highlight the captured clicked point.
     * @param _TFPoint the selected TF point.
     */
    void leftButtonClickOnPointEvent(std::pair< Point2DType, QGraphicsEllipseItem* >& _TFPoint);

    /**
     * @brief Move @ref m_capturedTFPoint to the new mouse position and update the TF.
     * @param _event the 2D scene event.
     *
     * @pre m_capturedTFPoint must be previously sets.
     * @see leftButtonClickOnPointEvent(std::pair< Point2DType, QGraphicsEllipseItem* >&)
     */
    void mouseMoveOnPointEvent(const ::fwRenderQt::data::Event& _event);

    /**
     * @brief Resets the captured TF point highlighting and sets @ref m_capturedTFPoint to null.
     *
     * @pre m_capturedTFPoint must be previously sets.
     * @see leftButtonClickOnPointEvent(std::pair< Point2DType, QGraphicsEllipseItem* >&)
     */
    void leftButtonReleaseEvent();

    /**
     * @brief Changes the TF point color by opening a color dialog and update the TF.
     * @param _TFPoint the selected TF point.
     */
    void leftButtonDoubleClickOnPointEvent(std::pair< Point2DType, QGraphicsEllipseItem* >& _TFPoint);

    /**
     * @brief Removes a TF point from the TF.
     * @param _TFPoint the selected TF point.
     */
    void rightButtonClickOnPointEvent(std::pair< Point2DType, QGraphicsEllipseItem* >& _TFPoint);

    /**
     * @brief Adds a new TF point to the TF.
     * @param _event the 2D scene event.
     */
    void leftButtonDoubleClickEvent(const ::fwRenderQt::data::Event& _event);

    /**
     * @brief Sets @ref m_capturedTF if the clicked coord if over the TF.
     * @param _event the 2D scene event.
     */
    void midButtonClickEvent(const ::fwRenderQt::data::Event& _event);

    /**
     * @brief Update the window/level of the TF relativly to the mouse movement.
     * @param _event the 2D scene event.
     *
     * @pre m_capturedTF must be previously sets.
     * @see midButtonClickEvent(const ::fwRenderQt::data::Event&)
     */
    void mouseMoveOnTFEvent(const ::fwRenderQt::data::Event& _event);

    /**
     * @brief Resets @ref m_capturedTF.
     *
     * @pre m_capturedTF must be previously sets.
     * @see midButtonClickEvent(const ::fwRenderQt::data::Event&)
     */
    void midButtonReleaseEvent();

    /**
     * @brief Open a context menu.
     * @param _event the 2D scene event.
     */
    void rightButtonCLickEvent(const ::fwRenderQt::data::Event& _event);

    /**
     * @brief Sets if the TF is clamped or not.
     * @param _clamp the clamp status.
     */
    void clampTF(bool _clamp);

    /**
     * @brief Sets if the TF interpolation mode is linear or nearest.
     * @param _linear uses true is the interpolation mode must be linear.
     */
    void toggleLinearTF(bool _linear);

    /// Defines the size of TF points in a ratio relative to the window.
    float m_pointSize { 0.03f };

    /// Defines the pen used by gradients.
    QPen m_polygonsPen;

    /// Defines the pen used by TF points.
    QPen m_pointsPen;

    /// Sets if interactions are enable or not.
    bool m_interactive { true };

    /// Stores the main layer.
    QGraphicsItemGroup* m_layer;

    /// Contains a set of graphic point and it coordinate in the window/level space.
    std::vector< std::pair< Point2DType, QGraphicsEllipseItem* > > m_TFPoints;

    /// Contains the graphic gradient.
    QGraphicsPolygonItem* m_TFPolygon { nullptr };

    /// Stores the captured clicked point.
    std::pair< Point2DType, QGraphicsEllipseItem* >* m_capturedTFPoint { nullptr };

    /// Stores the captured clicked TF and the current mouse position,
    /// the first coord is in the window/level space and the second in screen space,
    /// it allows to adjust the window/level of the current TF.
    std::pair< ::fwData::TransferFunction::sptr, ::fwRenderQt::data::Coord > m_capturedTF;

};

} // namespace adaptor
} // namespace scene2D
