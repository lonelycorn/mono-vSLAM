#pragma once
#include <os/mutex.hpp>
#include <base/space.hpp>
#include <front-end/data-type.hpp>
#include <front-end/image-pair.hpp>
#include <unordered_map>
#include <deque>

namespace mvSLAM
{

/** Dead-reckoning with visual frames only.
 */
class VisualOdometer
{
    // currently we use only the latest frame and the points
    // contained in it.
public:

    struct Params
    {
        // for descriptor matching
        uint32_t max_match_inlier_distance;

        // for initialization
        /** number of frames to hold in queue. Will try to find a good
         * image pair from two frames in the queue.
         * NOTE: must not be smaller than 2.
         */
        size_t frame_queue_size;
        uint32_t min_match_inlier_count;
        ScalarType max_error; // of the estimate
        ScalarType max_rotation_magnitude; // norm of so3
        ScalarType max_translation_z;

        // for tracking
        size_t min_tracked_point_match_count; // for pnp
        size_t min_tracked_point_count;

        // for bundle-adjustment
        ScalarType anchor_stddev_position;
        ScalarType anchor_stddev_orientation;
        ScalarType regulator_stddev_position;
        ScalarType regulator_stddev_orientation;
    };

    static Params get_default_params();

    VisualOdometer(const Params &p);
    ~VisualOdometer();

    /** Update visual odometer with a new frame.
     * @param [in] new_frame
     * @return true if the pose of the frame could be found
     */
    bool add_frame(const FrontEndTypes::FramePtr &new_frame);

    /// similar to @ref add_frame. provided for convenience
    bool add_frame_by_id(FrontEndTypes::FrameId new_frame_id);


    /// reset to initial state. keep the same params.
    void reset();

    /// return true if initialized.
    bool initialized() const;

    /** transformation from the BODY ref frame of latest frame to the BODY
     * ref frame of last initialization, i.e. T_body_to_world
     * NOTE: only call after initialized.
     */
    Transformation get_body_pose() const;

    /** transformation from the CAMERA ref frame of latest frame to the BODY
     * ref frame of last initialization, i.e. T_camera_to_world
     * NOTE: only call after initialized.
     */
    Transformation get_camera_pose() const;

    /** point positions expressed in the BODY ref frame of last initialization.
     * NOTE: only call after initialized.
     */
    std::vector<Point3> get_tracked_points() const;

private:

    /** Try to find a good image pair to start tracking
     */
    bool initialize(const FrontEndTypes::FramePtr &new_frame);

    /// return true if @p ip is good for initialization
    bool check_image_pair(const ImagePair &ip) const;

    /** Try to find the pose of @p new_frame.
     */
    bool track(const FrontEndTypes::FramePtr &new_frame);

    const Params m_params;

    mutable Mutex m_mutex;

    enum class State
    {
        INITIALIZING, ///< trying to find a good image pair
        TRACKING, ///< normal operation
    };
    State m_state;

    // Truely, we should use the descriptor as the key to index everything.
    // however, that requires a good hash function for 256-bit integer,
    // or a KD-tree.

    // TODO: maybe move these into FrontEndTypes
    using PointId = Id::Type;
    PointId generate_point_id();

    // use when INITIALIZING
    std::deque<FrontEndTypes::FramePtr> m_frame_queue;
    std::deque<ImagePair> m_image_pair_queue;

    // use when TRACKING
    FrontEndTypes::FramePtr m_last_frame;
    /** Transformation from the camera ref frame of last initialization to
     * the camera ref frame of the latest frame. i.e. T_C0_to_Cn
     * Saving this instead of its inverse to save time converting points.
     */
    Transformation T_init_frame_to_last_frame;
    /// coordinates of tracked points in init ref frame.
    std::unordered_map<PointId, Point3> m_point_id_to_point3;
    /// indexes of the reconstructed points in latest frame's VisualFeature
    std::unordered_map<PointId, size_t> m_point_id_to_last_frame_vf_idx;
    /// inverse mapping of @ref m_point_id_to_last_frame_vf_idx;
    std::unordered_map<size_t, PointId> m_last_frame_vf_idx_to_point_id;
};

};

