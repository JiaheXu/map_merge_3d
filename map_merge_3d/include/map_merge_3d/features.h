#ifndef MAP_MERGE_FEATURES_H_
#define MAP_MERGE_FEATURES_H_

#include <map_merge_3d/enum.h>
#include <map_merge_3d/typedefs.h>

namespace map_merge_3d
{
/**
 * @defgroup features Features
 * @brief Low-level feature-extracting functions.
 * @details Low-level functions to extract features from pointclouds and to
 * preprocess pointclouds for feature extraction. These function operate on
 * individual pointcloud.
 * @{
 */

/// @cond DOXYGEN_SKIP
// define this for dispatch
#define DESCRIPTORS_NAMES_ PFH, FPFH, RSD, SHOT, SC3D
/// @endcond DOXYGEN_SKIP

// define enum class Descriptor + string conversions
ENUM_CLASS(Descriptor, DESCRIPTORS_NAMES_);

/**
 * @brief Voxelize input pointcloud to reduce number of points.
 *
 * @param input input pointcloud
 * @param resolution required resolution for voxelization
 *
 * @return Voxelized pointcloud
 */
PointCloudPtr downSample(const PointCloudConstPtr &input, double resolution);

/**
 * @brief Removes outliers from the pointcloud
 * @details Outliers with small number of neighbours will be removed.
 *
 * @param input input pointcloud
 * @param radius Area where neighbours will be counted
 * @param min_neighbours Minimal number of neighbours for the point to be kept
 * @return filtered pointcloud
 */
PointCloudPtr removeOutliers(const PointCloudConstPtr &input, double radius,
                             int min_neighbours);

/**
 * @brief Filters points by bounding heights in pointcloud
 * @details Points that lie outside the bounding heights are removed.
 *
 * @param input input pointcloud
 * @param z_min Minimal z-height for the point to be kept
 * @param z_max Maximum z-height for the point to be kept
 * @return filtered pointcloud by height
 */
PointCloudPtr filterHeight(const PointCloudConstPtr &input, double z_min,
                           double z_max);

PointCloudPtr insertErrorIntoSourceCloud(const PointCloudConstPtr &src_cloud);

Eigen::Affine3f getXYPlaneParallelTransform(const PointCloudPtr &input);

// define enum class Keypoint + string conversions
ENUM_CLASS(Keypoint, SIFT, HARRIS, ISS);

/**
 * @brief Detects keypoints in the pointcloud
 * @details Normals are used only for geometric keypoints (HARRIS). SIFT
 * keypoints requires valid colour information with points.
 *
 * @param points input pointcloud
 * @param normals normals for input
 * @param type keypoint type to extract
 * @param threshold determines how much keypoints to extract (corner measure)
 * @param radius area used for keypoint detection around each point
 * @param resolution smallest scale of the scale pyramid if the detector uses
 * one
 * @return pointcloud of keypoints
 */
PointCloudPtr detectKeypoints(const PointCloudConstPtr &points,
                              const SurfaceNormalsPtr &normals, Keypoint type,
                              double threshold, double radius,
                              double resolution);

/**
 * @brief Compute local feature descriptors around each keypoint.
 * @details If descriptors can not be computed to some of the keypoints, those
 * keypoints will be removed from the keypoints cloud. Therefore the keypoints
 * cloud can be also modified.
 *
 * @param points input pointcloud
 * @param normals input normals for the cloud
 * @param keypoints input detected keypoints, where descriptors will be computed
 * @param descriptor descriptor type to extract
 * @param feature_radius search radius for descriptors
 * @return cloud of local descriptors
 */
LocalDescriptorsPtr computeLocalDescriptors(const PointCloudConstPtr &points,
                                            const SurfaceNormalsPtr &normals,
                                            const PointCloudPtr &keypoints,
                                            Descriptor descriptor,
                                            double feature_radius);

/**
 * @brief Estimate cloud surface normals
 *
 * @param input input cloud
 * @param radius local neighbourhood size for estimating normals
 *
 * @return cloud of computed normals
 */
SurfaceNormalsPtr computeSurfaceNormals(const PointCloudConstPtr &input,
                                        double radius);

///@} group features

}  // namespace map_merge_3d

#endif  // MAP_MERGE_FEATURES_H_
