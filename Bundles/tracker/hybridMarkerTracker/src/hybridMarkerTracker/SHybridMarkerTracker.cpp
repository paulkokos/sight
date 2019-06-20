/************************************************************************
 *
 * Copyright (C) 2019 IRCAD France
 * Copyright (C) 2019 IHU Strasbourg
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

#include "hybridMarkerTracker/SHybridMarkerTracker.hpp"

#include <arData/Camera.hpp>
#include <arData/FrameTL.hpp>
#include <arData/MarkerMap.hpp>
#include <arData/MarkerTL.hpp>

#include <cvIO/Camera.hpp>
#include <cvIO/FrameTL.hpp>
#include <cvIO/Image.hpp>

#include <fwCom/Signal.hxx>
#include <fwCom/Slots.hxx>

#include <fwData/Image.hpp>
#include <fwData/mt/ObjectReadToWriteLock.hpp>

#include <fwRuntime/operations.hpp>

#include <cmath>
namespace hybridMarkerTracker
{

fwServicesRegisterMacro(::arServices::ITracker, ::hybridMarkerTracker::SHybridMarkerTracker);

static const ::fwServices::IService::KeyType s_TIMELINE_INOUT = "frameTL";
static const ::fwServices::IService::KeyType s_IMAGE_INOUT    = "frameOut";

SHybridMarkerTracker::SHybridMarkerTracker() noexcept :
    m_tracker(NULL)
{
}

SHybridMarkerTracker::~SHybridMarkerTracker()
{
    if(!m_tracker)
    {
        delete m_tracker;
    }
}
//------------------------------------------------------------------------------

void SHybridMarkerTracker::readSettings(std::string filename)
{
    std::cout << "Initializing..." << std::endl;
    m_fs.open(filename, ::cv::FileStorage::READ);
    // Read settings & configuration
    m_fs["patternToUse" ] >> m_patternToUse;
    m_fs["Sym_BoardSize_Width" ] >> m_symboardSize.width;
    m_fs["Sym_BoardSize_Height"] >> m_symboardSize.height;
    m_fs["Square_Size"]  >> m_squareSize;
    m_fs["Asym_Square_Size"]  >> m_asymSquareSize;
    m_fs["Sym_Square_Size_X" ] >> m_symSquareSize.x;
    m_fs["Sym_Square_Size_Y"] >> m_symSquareSize.y;
    m_fs["Radius"]  >> m_radius;
    m_fs["Chess_Dist_Center"]  >> m_chessDistCenter;
    m_fs["Chess_Interval"]  >> m_chessInterval;

    m_fs["image_Width" ] >> m_imgSize.width;
    m_fs["image_Height"] >> m_imgSize.height;
    m_fs["Camera_Matrix"] >> m_cameraMatrix;
    m_fs["Distortion_Coefficients"] >> m_distCoeffs;

    m_fs.release();

    // --- Tracker parameters---
    ::cv::Size roi_size(300, 300);
    // Global blob detector setting
    ::cv::SimpleBlobDetector::Params params;
    params.minRepeatability    = 2;
    params.minDistBetweenBlobs = 10;
    params.minThreshold        = 80;
    params.maxThreshold        = 160;
    params.thresholdStep       = 20;
    params.filterByArea        = true;
    params.minArea             = 50;
    params.maxArea             = 1000;
    params.filterByConvexity   = true;
    params.minConvexity        = 0.85f;
    params.maxConvexity        = 1.0f;
    params.filterByCircularity = true;
    params.minCircularity      = 0.7f;
    params.maxCircularity      = 1.0f;
    params.filterByInertia     = false;
    params.minInertiaRatio     = 0.01;

    // Local blob detector setting
    ::cv::SimpleBlobDetector::Params params_roi;
    params_roi.minRepeatability    = 2;
    params_roi.minDistBetweenBlobs = 10;
    params_roi.minThreshold        = 50;
    params_roi.maxThreshold        = 220;
    params_roi.thresholdStep       = 10;
    params_roi.filterByArea        = true;
    params_roi.minArea             = 50;
    params_roi.maxArea             = 2000;
    params.filterByConvexity       = true;
    params.minConvexity            = 0.85f;
    params.maxConvexity            = 1.0f;
    params.filterByCircularity     = true;
    params.minCircularity          = 0.75;
    params.maxCircularity          = 1.0f;

    /**
     *  Calculate model points for marker
     */
    if (m_patternToUse.compare("HYBRID") == 0)
    {
        /************************************************************************/
        /* Hybrid pattern                                                       */
        /************************************************************************/
        float arc_inner      = m_symSquareSize.y;
        float arc_outter     = arc_inner + 2 * m_asymSquareSize;
        float chord_inner_2  = m_radius * sin(arc_inner / (2*m_radius)); // chord/2
        float chord_outter_2 = m_radius * sin(arc_outter / (2*m_radius));
        float sagitta_inner  = m_radius - sqrt(m_radius * m_radius - chord_inner_2 * chord_inner_2);
        float sagitta_outter = m_radius - sqrt(m_radius * m_radius - chord_outter_2 * chord_outter_2);
        // MID
        for ( int i = 0; i < m_symboardSize.height; i++ )
        {
            for ( int j = 0; j < m_symboardSize.width; j++)
            {
                ::cv::Point3f pt;
                pt.x = i * m_symSquareSize.x;
                pt.y = chord_inner_2;
                pt.y = (j % 2) == 0 ? pt.y : -pt.y;
                pt.z = sagitta_inner;
                m_trackMidPatternPoints.push_back(pt);
            }
        }
        // TOP
        int asym_points_num = m_symboardSize.height + m_symboardSize.height - 1;
        for ( int i = 0; i < asym_points_num; i++ )
        {
            ::cv::Point3f pt;
            pt.x = (m_symboardSize.height-1) * m_symSquareSize.x
                   - i * m_asymSquareSize;
            pt.y = (i % 2) == 0 ? chord_inner_2 : chord_outter_2;
            pt.z = (i % 2) == 0 ? sagitta_inner : sagitta_outter;
            m_trackTopPatternPoints.push_back(pt);
        }
        // BOT
        for ( int i = 0; i < asym_points_num; i++ )
        {
            ::cv::Point3f pt;
            pt.x = i * m_asymSquareSize;
            pt.y = (i % 2) == 0 ? -chord_inner_2 : -chord_outter_2;
            pt.z = (i % 2) == 0 ? sagitta_inner : sagitta_outter;
            m_trackBotPatternPoints.push_back(pt);
        }

        // Chess pattern points
        // MID
        for (auto i = -1; i < 5; i++)
        {
            ::cv::Point3f pt(0.0f, 0.0f, 0.0f);
            pt.x = m_chessInterval/2 + i * m_chessInterval;
            m_trackChessMidPatternPoint.push_back(pt);
        }

        float arc_chess     = m_chessDistCenter * 2;
        float chord_chess_2 = m_radius * sin(arc_chess / (2*m_radius));
        float sagitta_chess;
        if (arc_chess < CV_PI * m_radius)
        {
            sagitta_chess = m_radius - sqrt(m_radius * m_radius - chord_chess_2 * chord_chess_2);
        }
        else
        {
            sagitta_chess = m_radius + sqrt(m_radius * m_radius - chord_chess_2 * chord_chess_2);
        }

        // TOP
        for (auto i = 0; i < 5; i++)
        {
            ::cv::Point3f pt;
            pt.x = i * m_chessInterval;
            pt.y = chord_chess_2;
            pt.z = sagitta_chess;

            m_trackChessTopPatternPoint.push_back(pt);
        }

        // BOT
        for (auto i = 0; i < 5; i++)
        {
            ::cv::Point3f pt;
            pt.x = i * m_chessInterval;
            pt.y = -chord_chess_2;
            pt.z = sagitta_chess;

            m_trackChessBotPatternPoint.push_back(pt);
        }
        m_tracker = new TrackerCurvedot(m_symboardSize, roi_size, params, params_roi);
    }

    else
    {
        std::cerr << "Unknow pattern type" << std::endl;
        exit(0);
    }
}
//------------------------------------------------------------------------------

void SHybridMarkerTracker::process(const ::cv::Mat& img, ::cv::Mat& out_img)
{
    m_imgTrack = img;

    bool use_ippe = true;
    // Tracking
    if(m_patternToUse.compare("HYBRID") == 0)
    {
        if (static_cast<TrackerCurvedot*>(m_tracker)->track(m_imgTrack))
        {
            std::vector< ::cv::Point2f > imgPoints = static_cast< TrackerCurvedot* >(m_tracker)->getP_img();

            // Determine which pattern is detected
            std::vector< ::cv::Point3f > visiblePatternPoints;
            int curr_detect_state = static_cast<TrackerCurvedot*>(m_tracker)->CurrDetectState();
            if ((curr_detect_state& TrackerCurvedot::TOP_CIR) && !(curr_detect_state & TrackerCurvedot::MID_CIR))
            {
                visiblePatternPoints = m_trackTopPatternPoints;
            }
            else if ((curr_detect_state& TrackerCurvedot::BOT_CIR) && !(curr_detect_state & TrackerCurvedot::MID_CIR))
            {
                visiblePatternPoints = m_trackBotPatternPoints;
            }
            else if ((curr_detect_state& TrackerCurvedot::TOP_CIR) && (curr_detect_state & TrackerCurvedot::MID_CIR))
            {
                int num_sym_dot = m_trackMidPatternPoints.size();
                visiblePatternPoints.reserve(m_trackTopPatternPoints.size() + num_sym_dot/2);
                visiblePatternPoints.insert(visiblePatternPoints.end(),
                                            m_trackTopPatternPoints.begin(), m_trackTopPatternPoints.end());
                for (int i = 0; i < num_sym_dot; i++)
                {
                    if (i%2 != 0)
                    {
                        visiblePatternPoints.push_back(m_trackMidPatternPoints[i]);
                    }
                }
                use_ippe = false;   // IPPE is only for planar model
            }
            else if ((curr_detect_state& TrackerCurvedot::BOT_CIR) && (curr_detect_state & TrackerCurvedot::MID_CIR))
            {
                int num_sym_dot = m_trackMidPatternPoints.size();
                visiblePatternPoints.reserve(m_trackBotPatternPoints.size() + num_sym_dot/2);
                visiblePatternPoints.insert(visiblePatternPoints.end(),
                                            m_trackBotPatternPoints.begin(), m_trackBotPatternPoints.end());
                for (int i = 0; i < num_sym_dot; i++)
                {
                    if (i%2 == 0)
                    {
                        visiblePatternPoints.push_back(m_trackMidPatternPoints[i]);
                    }
                }
                use_ippe = false;
            }
            else if (curr_detect_state & TrackerCurvedot::MID_CIR)
            {
                visiblePatternPoints = m_trackMidPatternPoints;
            }

            // Calculate pattern pose in camera coordinate
            ::cv::Mat rvec, tvec, cRp;
            ::cv::Mat rvec1, tvec1; // 1st solution
            ::cv::Mat rvec2, tvec2; // 2nd solution
            float error1, error2;

            m_currentcHp = ::cv::Mat::eye(4, 4, CV_64F);

            if (use_ippe)
            {
                std::vector< ::cv::Point3f > pts_3d;
                bool is_chess_detect = true;

                if (curr_detect_state & TrackerCurvedot::TOP_CHESS)
                {
                    pts_3d = m_trackChessTopPatternPoint;
                }
                else if (curr_detect_state & TrackerCurvedot::MID_CHESS)
                {
                    pts_3d = m_trackChessMidPatternPoint;
                }
                else if (curr_detect_state & TrackerCurvedot::BOT_CHESS)
                {
                    pts_3d = m_trackChessBotPatternPoint;
                }
                else
                {
                    is_chess_detect = false;    // Chess line not found

                }
                ippeSolver.solveGeneric(visiblePatternPoints, imgPoints,
                                        m_cameraMatrix, m_distCoeffs, rvec1, tvec1, error1, rvec2, tvec2, error2);

                // Use chessboard features to disambigulate if two solutions are similar
                if (is_chess_detect && abs(error1 - error2) < 0.1 && error1 < 0.2 && error2 < 0.2)
                {
                    calculateCorrectPose(rvec1, tvec1, rvec2, tvec2, pts_3d, rvec, tvec);
                }
                else
                {
                    rvec = rvec1;
                    tvec = tvec1;
                }

                ::cv::Mat cHp_1 = ::cv::Mat::eye(4, 4, CV_64F);
                ::cv::Rodrigues(rvec1, cRp);
                ::cv::Mat aux = cHp_1.colRange(0, 3).rowRange(0, 3);
                cRp.copyTo(aux);
                aux = cHp_1.colRange(3, 4).rowRange(0, 3);
                tvec1.copyTo(aux);

                ::cv::Mat cHp_2 = ::cv::Mat::eye(4, 4, CV_64F);
                ::cv::Rodrigues(rvec2, cRp);
                aux = cHp_2.colRange(0, 3).rowRange(0, 3);
                cRp.copyTo(aux);
                aux = cHp_2.colRange(3, 4).rowRange(0, 3);
                tvec2.copyTo(aux);

                drawRect(cHp_1, m_imgTrack, ::cv::Scalar(0, 0, 255));
                drawRect(cHp_2, m_imgTrack, ::cv::Scalar(255, 0, 0));
            }
            else
            {
                ::cv::solvePnP(visiblePatternPoints, imgPoints, m_cameraMatrix, m_distCoeffs, rvec, tvec);
                ::cv::Rodrigues(rvec, cRp);
                ::cv::Mat aux = m_currentcHp.colRange(0, 3).rowRange(0, 3);
                cRp.copyTo(aux);
                aux = m_currentcHp.colRange(3, 4).rowRange(0, 3);
                tvec.copyTo(aux);
                drawRect(m_currentcHp, m_imgTrack);
            }

            static_cast<TrackerCurvedot*>(m_tracker)->drawKeydots(m_imgTrack);
        }

    }

    std::string str_1 = "Blue rectangle shows current estimated pose";
    std::string str_2 = "Red rectangle shows the ambiguous pose provided by IPPE";
    std::string str_3 = "Green shows detection of pattern";
    std::string str_4 = "Yellow shows tracking of pattern";
    ::cv::putText(m_imgTrack, str_1, ::cv::Point(10, 20), ::cv::FONT_HERSHEY_COMPLEX, 0.5, ::cv::Scalar(0, 0, 255), 1);
    ::cv::putText(m_imgTrack, str_2, ::cv::Point(10, 40), ::cv::FONT_HERSHEY_COMPLEX, 0.5, ::cv::Scalar(255, 0, 0), 1);
    ::cv::putText(m_imgTrack, str_3, ::cv::Point(10, 60), ::cv::FONT_HERSHEY_COMPLEX, 0.5, ::cv::Scalar(0, 255, 0), 1);
    ::cv::putText(m_imgTrack, str_4, ::cv::Point(10, 80), ::cv::FONT_HERSHEY_COMPLEX, 0.5, ::cv::Scalar(255, 255, 0),
                  1);

    m_imgTrack.copyTo(out_img);
}

//------------------------------------------------------------------------------

cv::Vec2f SHybridMarkerTracker::errorDistPoints(const std::vector<cv::Point2f>& pts_d,
                                                const std::vector<cv::Point2f>& pts_1,
                                                const std::vector<cv::Point2f>& pts_2,
                                                const double max_dist_sq)
{
    cv::Vec2f sum_error(0, 0);
    auto dist_sq_1 = 0.0, dist_sq_2 = 0.0;
    cv::Point2f pts_diff;
    for (auto i = 0; i < pts_d.size(); i++)
    {
        int valid_ind = -1;
        for (auto j = 0; j < pts_1.size(); j++)
        {
            pts_diff  = pts_d[i] - pts_1[j];
            dist_sq_1 = pts_diff.x * pts_diff.x + pts_diff.y * pts_diff.y;
            if (dist_sq_1 > max_dist_sq)
            {
                continue;
            }
            else
            {
                valid_ind = j;
                break;
            }
        }

        for (auto j = 0; j < pts_2.size(); j++)
        {
            pts_diff  = pts_d[i] - pts_2[j];
            dist_sq_2 = pts_diff.x * pts_diff.x + pts_diff.y * pts_diff.y;
            if (dist_sq_2 > max_dist_sq)
            {
                continue;
            }

            if (valid_ind == j) // assume pts_1 and pts_2 have same order
            {
                sum_error[0] += dist_sq_1;
                sum_error[1] += dist_sq_2;

                break;
            }
        }
        dist_sq_1 = dist_sq_2 = 0.0;
    }

    // recursive in case threshold max_dist_sq is too small
    if (sum_error[0] == 0.0)
    {
        sum_error = errorDistPoints(pts_d, pts_1, pts_2, max_dist_sq * 2);
    }

    return sum_error;
}

//------------------------------------------------------------------------------

void SHybridMarkerTracker::calculateCorrectPose(
    ::cv::InputArray rvec1, ::cv::InputArray tvec1,
    ::cv::InputArray rvec2, ::cv::InputArray tvec2,
    const std::vector< ::cv::Point3f >& pts_3d,
    ::cv::OutputArray rvec, ::cv::OutputArray tvec)
{
    std::vector<cv::Point2f> projPoints_1, projPoints_2;

    cv::projectPoints(pts_3d, rvec1, tvec1,
                      m_cameraMatrix, m_distCoeffs, projPoints_1);
    cv::projectPoints(pts_3d, rvec2, tvec2,
                      m_cameraMatrix, m_distCoeffs, projPoints_2);

    std::vector<cv::Point2f> detect_pts = static_cast<TrackerCurvedot*>(m_tracker)->get_chess_pts();

    // Calculate a threshold to determine correspondence
    cv::Point2f diff_temp = (projPoints_1[0] - projPoints_1[1]) * 0.7;
    double max_dist_sq    = diff_temp.x*diff_temp.x + diff_temp.y*diff_temp.y;
    diff_temp   = (projPoints_2[0] - projPoints_2[1]) * 0.7;
    max_dist_sq = (max_dist_sq + diff_temp.x*diff_temp.x + diff_temp.y*diff_temp.y)/2;

    cv::Vec2f errors = errorDistPoints(detect_pts, projPoints_1, projPoints_2, max_dist_sq);

    rvec.create(3, 1, CV_64FC1);
    tvec.create(3, 1, CV_64FC1);
    if ( errors[0] < errors[1] )
    {
        rvec1.getMat().copyTo(rvec);
        tvec1.getMat().copyTo(tvec);
    }
    else
    {
        rvec2.getMat().copyTo(rvec);
        tvec2.getMat().copyTo(tvec);
    }
}

//------------------------------------------------------------------------------

void SHybridMarkerTracker::drawRect(const cv::Mat& cHp, cv::Mat& img, cv::Scalar color)
{
    cv::Mat rect_corners = cv::Mat::ones(4, 4, CV_64F);
    rect_corners.at<double>(0, 0) = 0.0;
    rect_corners.at<double>(1, 0) = 0.0;
    rect_corners.at<double>(2, 0) = 0.0;
    rect_corners.at<double>(0, 1) = 20.0;
    rect_corners.at<double>(1, 1) = 0.0;
    rect_corners.at<double>(2, 1) = 0.0;
    rect_corners.at<double>(0, 2) = 20.0;
    rect_corners.at<double>(1, 2) = 0.0;
    rect_corners.at<double>(2, 2) = 30.0;
    rect_corners.at<double>(0, 3) = 0.0;
    rect_corners.at<double>(1, 3) = 0.0;
    rect_corners.at<double>(2, 3) = 30.0;

    cv::Mat corners_hm = cHp * rect_corners;

    std::vector <cv::Point3f> corners(4);    // 0-3: US image endpoints, 4-7: shadow pts

    for (int i = 0; i < corners.size(); i++)
    {
        corners[i].x = corners_hm.at<double>(0, i) / corners_hm.at<double>(3, i);
        corners[i].y = corners_hm.at<double>(1, i) / corners_hm.at<double>(3, i);
        corners[i].z = corners_hm.at<double>(2, i) / corners_hm.at<double>(3, i);
    }

    cv::Mat rVec, tVec, m_distCoeffs;
    rVec = cv::Mat::zeros(3, 1, CV_32FC1);
    tVec = cv::Mat::zeros(3, 1, CV_32FC1);
    std::vector <cv::Point2f> corners_2d;

    cv::projectPoints(corners, rVec, tVec, m_cameraMatrix,
                      m_distCoeffs, corners_2d);

    cv::Rodrigues(cHp(cv::Rect(0, 0, 3, 3)), rVec);
    cHp.colRange(3, 4).rowRange(0, 3).copyTo(tVec);

    cv::line(img, corners_2d[0], corners_2d[1], color, 2);
    cv::line(img, corners_2d[1], corners_2d[2], color, 2);
    cv::line(img, corners_2d[2], corners_2d[3], color, 2);
    cv::line(img, corners_2d[0], corners_2d[3], color, 2);

    cv::circle(img, corners_2d[0], 4, color, -1);

}

//------------------------------------------------------------------------------

::fwServices::IService::KeyConnectionsMap SHybridMarkerTracker::getAutoConnections() const
{
    KeyConnectionsMap connections;
    connections.push( s_TIMELINE_INOUT, ::arData::TimeLine::s_OBJECT_PUSHED_SIG, s_TRACK_SLOT );
    connections.push( s_FRAME_INOUT, ::fwData::Object::s_MODIFIED_SIG, s_UPDATE_SLOT );
    connections.push( s_FRAME_INOUT, ::fwData::Image::s_BUFFER_MODIFIED_SIG, s_UPDATE_SLOT );
    return connections;
}

//------------------------------------------------------------------------------

void SHybridMarkerTracker::starting()
{
    auto filePath = ::fwRuntime::getBundleResourceFilePath("hybridMarkerTracker", "settings.xml");
    readSettings(filePath.string());
    this->startTracking();
}

//------------------------------------------------------------------------------

void SHybridMarkerTracker::tracking(::fwCore::HiResClock::HiResClockType& timestamp)
{
    auto frameTL = this->getInOut< ::arData::FrameTL >(s_TIMELINE_INOUT);
    SLM_ASSERT("The InOut key '" + s_TIMELINE_INOUT + "' is not defined", frameTL);

    auto frameOutput = this->getInOut< ::fwData::Image >(s_IMAGE_INOUT);
    SLM_ASSERT("The InOut key '" + s_IMAGE_INOUT + "' is not defined", frameOutput);

    ::cv::Mat img, img_track;
    if (frameTL)
    {
        const CSPTR(::arData::FrameTL::BufferType)
        buffer = frameTL->getClosestBuffer(timestamp);
        std::uint8_t* frameBuff = const_cast< std::uint8_t*>( &buffer->getElement(0));

        ::cvIO::FrameTL::moveToCv(frameTL, frameBuff, img);
        process(img, img_track);

//        check if image dimension have changed
        ::fwData::Image::SizeType size(2);
        size[0] = frameTL->getWidth();
        size[1] = frameTL->getHeight();

        if(size != frameOutput->getSize())
        {
            m_imagesInitialized = false;
        }

        if(!m_imagesInitialized)
        {
            const ::fwData::Image::SpacingType::value_type voxelSize = 1;
            frameOutput->allocate(size, frameTL->getType(), frameTL->getNumberOfComponents());
            ::fwData::Image::OriginType origin(2, 0);

            frameOutput->setOrigin(origin);
            ::fwData::Image::SpacingType spacing(2, voxelSize);
            frameOutput->setSpacing(spacing);
            frameOutput->setWindowWidth(1);
            frameOutput->setWindowCenter(0);

            m_imagesInitialized = true;

            auto sig = frameOutput->signal< ::fwData::Image::BufferModifiedSignalType >(
                ::fwData::Image::s_MODIFIED_SIG );
            sig->asyncEmit();
        }
        ::cvIO::Image::copyFromCv(frameOutput, img_track);
        auto sig = frameOutput->signal< ::fwData::Object::ModifiedSignalType >(::fwData::Image::s_BUFFER_MODIFIED_SIG);
        sig->asyncEmit();
    }
}
//------------------------------------------------------------------------------

void SHybridMarkerTracker::configuring()
{
}
//------------------------------------------------------------------------------

void SHybridMarkerTracker::updating()
{
}
//------------------------------------------------------------------------------

void SHybridMarkerTracker::stopping()
{
    this->stopTracking();
}
} // hybridMarkerTracker namespace
