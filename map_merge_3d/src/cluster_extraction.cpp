#include <map_merge_3d/cluster_extraction.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/features/normal_3d.h>
#include <pcl/search/kdtree.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/segmentation/extract_clusters.h>

namespace map_merge_3d
{
std::vector<PointCloudPtr> EuclideanClusterExtraction(const PointCloudConstPtr &cloud)
{

  // Read in the cloud data
  PointCloudPtr cloud_f (new PointCloud);

  // Create the filtering object: downsample the dataset using a leaf size of 1cm
  pcl::VoxelGrid<PointT> vg;
  pcl::PointCloud<PointT>::Ptr cloud_filtered (new pcl::PointCloud<PointT>);
  vg.setInputCloud (cloud);
  vg.setLeafSize (0.01f, 0.01f, 0.01f);
  vg.filter (*cloud_filtered);
  std::cout << "PointCloud after filtering has: " << cloud_filtered->size ()  << " data points." << std::endl; //*

  // Create the segmentation object for the planar model and set all the parameters
  pcl::SACSegmentation<PointT> seg;
  pcl::PointIndices::Ptr inliers (new pcl::PointIndices);
  pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients);
  pcl::PointCloud<PointT>::Ptr cloud_plane (new pcl::PointCloud<PointT> ());
  pcl::PCDWriter writer;
  seg.setOptimizeCoefficients (true);
  seg.setModelType (pcl::SACMODEL_PLANE);
  seg.setMethodType (pcl::SAC_RANSAC);
  seg.setMaxIterations (100);
  seg.setDistanceThreshold (0.02);

  int nr_points = (int) cloud_filtered->size ();
  int i = 1;
  while (cloud_filtered->size () > 0.3 * nr_points)
  {
    // Segment the largest planar component from the remaining cloud
    seg.setInputCloud (cloud_filtered);
    seg.segment (*inliers, *coefficients);
    if (inliers->indices.size () == 0)
    {
      std::cout << "Could not estimate a planar model for the given dataset." << std::endl;
      break;
    }

    // Extract the planar inliers from the input cloud
    pcl::ExtractIndices<PointT> extract;
    extract.setInputCloud (cloud_filtered);
    extract.setIndices (inliers);
    extract.setNegative (false);

    // Get the points associated with the planar surface
    extract.filter (*cloud_plane);
    std::cout << "PointCloud representing the planar component: " << cloud_plane->size () << " data points." << std::endl;

    // Remove the planar inliers, extract the rest
    extract.setNegative (true);
    extract.filter (*cloud_f);
    *cloud_filtered = *cloud_f;

    i++;
  }

  // Creating the KdTree object for the search method of the extraction
  pcl::search::KdTree<PointT>::Ptr tree (new pcl::search::KdTree<PointT>);
  tree->setInputCloud (cloud_filtered);

  /* Here we are creating a vector of PointIndices, which contains the actual index
  * information in a vector<int>. The indices of each detected cluster are saved here.
  * Cluster_indices is a vector containing one instance of PointIndices for each detected 
  * cluster. Cluster_indices[0] contain all indices of the first cluster in input point cloud.
  */
  std::vector<pcl::PointIndices> cluster_indices;
  pcl::EuclideanClusterExtraction<PointT> ec;
  ec.setClusterTolerance (0.5); // 2cm
  ec.setMinClusterSize (1000);
  ec.setMaxClusterSize (250000);
  ec.setSearchMethod (tree);
  ec.setInputCloud (cloud_filtered);
  ec.extract (cluster_indices);
  
  std::cout << "Found " << cluster_indices.size() << " clusters" << std::endl;

  /* To separate each cluster out of the vector<PointIndices> we have to 
  * iterate through cluster_indices, create a new PointCloud for each 
  * entry and write all points of the current cluster in the PointCloud. 
  */
  int j = 0;
  std::vector<PointCloudPtr> clusters;
  for (std::vector<pcl::PointIndices>::const_iterator it = cluster_indices.begin (); it != cluster_indices.end (); ++it)
  {
    PointCloudPtr cloud_cluster (new PointCloud);
    for (const auto& idx : it->indices)
      cloud_cluster->push_back ((*cloud_filtered)[idx]); //*
    cloud_cluster->width = cloud_cluster->size ();
    cloud_cluster->height = 1;
    cloud_cluster->is_dense = true;

    std::cout << "PointCloud representing the Cluster: " << cloud_cluster->size () << " data points." << std::endl;
    clusters.emplace_back(std::move(cloud_cluster));

    j++;

  }

  return clusters;

}

}  // namespace map_merge_3d

// int main(int argc, char** argv) {
//   PointCloudConstPtr input;

//   PointCloudPtr cloud (new PointCloud);
//   if (pcl::io::loadPCDFile<PointT> ("new_jackal_02.pcd", *cloud) == -1) //* load the file
//   {
//     PCL_ERROR ("Couldn't read file test_pcd.pcd \n");
//   }

//   input = cloud;

//   std::vector<PointCloudPtr> clusters;
//   clusters = EuclideanClusterExtraction(input);

//   PointCloudPtr cloud_all (new PointCloud);
//   for (auto cluster : clusters) {
//     *cloud_all += *cluster;
//   }
//   visualisePointCloud(cloud_all);

// }