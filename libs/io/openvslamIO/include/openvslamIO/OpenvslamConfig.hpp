/************************************************************************
 *
 * Copyright (C) 2019-2020 IRCAD France
 * Copyright (C) 2019-2020 IHU Strasbourg
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

#include "openvslamIO/config.hpp"

namespace openvslamIO
{

/**
 * @brief Handles Openvslam orb parameters.
 */
struct OPENVSLAMIO_CLASS_API OrbParams
{
    /// Maximum number of key points.
    unsigned int maxNumKeyPts = 2000;
    ///
    float scaleFactor = 1.2f;
    /// Number of levels
    unsigned int numLevels = 8;
    ///
    unsigned int iniFastThr = 20;
    ///
    unsigned int minFastThr = 7;

};

/**
 * @brief Handles Openvslam Initializer parameters (only used for monocular).
 */
struct OPENVSLAMIO_CLASS_API InitParams
{
    /// Number of Ransac Iterations.
    unsigned int numRansacIterations = 100;
    /// Minimum number of triangulated points.
    unsigned int minNumTriangulatedPts = 50;
    /// Parallax threshold  (in degree).
    float parallaxDegThr = 1.0;
    /// Reprojection Error threshold.
    float reprojErrThr = 4.0;
    /// Number of Iterations for module-Adjustement.
    unsigned int numBAIterations = 20;
    /// Initial scale factor.
    float scalingFactor = 1000.0;
};

/**
 * @brief Handles Openvslam tracking parameters.
 */
struct OPENVSLAMIO_CLASS_API TrackingParams
{
    /// Depth threshold
    double trueDepthThr = 40.0;
    /// Depthmap factor (pixel_value / depthmap_factor = true_depth)
    double depthmapFactor = 1.0;
};

} //namespace openvslamIO
