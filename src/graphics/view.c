#include "graphics/view.h"
#include "toolbox/common.h"

void
otho_camera_get_view_projection_matrix(OthoCamera* cam, mat4 view_proj_mat_return)
{
    if (cam->dirty)
    {
        glm_translate_make(
            cam->view_matrix,
            (vec3){ -(cam->position[0] - cam->size[0] / 2.f), -(cam->position[1] - cam->size[1] / 2.f), 0 });
        glm_scale(cam->view_matrix, cam->scale);
        glm_rotate_z(cam->view_matrix, -cam->rotation, cam->view_matrix);
        glm_mat4_inv(cam->view_matrix, cam->inv_view_matrix);
        cam->dirty = false;
    }
    glm_mat4_mul(cam->projection_matrix, cam->view_matrix, view_proj_mat_return);
}

void
otho_camera_get_projection_matrix(OthoCamera* cam, mat4 proj_matrix_return)
{
    glm_mat4_copy(cam->projection_matrix, proj_matrix_return);
}

void
otho_camera_get_invert_view_matrix(OthoCamera* cam, mat4 invertViewMatReturn)
{
    if (cam->dirty)
    {
        glm_translate_make(
            cam->view_matrix,
            (vec3){ -(cam->position[0] - cam->size[0] / 2.f), -(cam->position[1] - cam->size[1] / 2.f), 0 });
        glm_scale(cam->view_matrix, cam->scale);
        glm_rotate_z(cam->view_matrix, -glm_rad(cam->rotation), cam->view_matrix);
        glm_mat4_inv(cam->view_matrix, cam->inv_view_matrix);
        cam->dirty = false;
    }
    glm_mat4_copy(cam->inv_view_matrix, invertViewMatReturn);
}

void
otho_camera_translate(OthoCamera* cam, vec2 v)
{
    cam->dirty = true;
    cam->position[0] += v[0];
    cam->position[1] += v[1];
}

void
otho_camera_rotate(OthoCamera* cam, float angle)
{
    cam->dirty = true;
    cam->rotation += angle;
}

void
otho_camera_zoom(OthoCamera* cam, vec2 v)
{
    cam->dirty = true;
    cam->scale[0] *= v[0];
    cam->scale[1] *= v[1];
}

void
otho_camera_set_position(OthoCamera* cam, vec2 pos)
{
    cam->dirty = true;
    glm_vec3_copy((vec3){ pos[0], pos[1], 0.f }, cam->position);
}

void
otho_camera_set_rotation(OthoCamera* cam, float rot)
{
    cam->dirty    = true;
    cam->rotation = rot;
}

void
otho_camera_set_zoom(OthoCamera* cam, float* z)
{
    cam->scale[0] = z[0];
    cam->scale[1] = z[1];
    cam->dirty    = true;
}

void
otho_camera_reset(OthoCamera* cam, float x, float y, float w, float h)
{
    cam->scale[0] = 1.f;
    cam->scale[1] = 1.f;
    cam->scale[2] = 1.f;

    cam->size[0] = w;
    cam->size[1] = h;

    cam->position[0] = x;
    cam->position[1] = y;

    cam->rotation = 0.f;
    cam->dirty    = true;

    glm_ortho(0, w, h, 0, 100, -100.f, cam->projection_matrix);
}

float
otho_camera_view_top(OthoCamera* cam)
{
    cam->dirty = true;
    return cam->position[1] - cam->size[1] / 2.f;
}

float
otho_camera_view_bot(OthoCamera* cam)
{
    cam->dirty = true;
    return cam->position[1] + cam->size[1] / 2.f;
}

float
otho_camera_view_left(OthoCamera* cam)
{
    cam->dirty = true;
    return cam->position[0] - cam->size[0] / 2.f;
}

float
otho_camera_view_right(OthoCamera* cam)
{
    cam->dirty = true;
    return cam->position[0] + cam->size[0] / 2.f;
}

void
otho_camera_to_view_coords(OthoCamera* cam, vec2 src, vec2 dst)
{
    vec4 tmp;
    glm_mat4_mulv(cam->inv_view_matrix, (vec4){ src[0], src[1], 0.f, 1.f }, tmp);
    dst[0] = tmp[0];
    dst[1] = tmp[1];
}

