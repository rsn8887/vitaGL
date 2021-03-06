/* 
 * misc.c:
 * Implementation for miscellaneous functions
 */

 #include "shared.h"
 
 static void update_polygon_offset(){
	switch (polygon_mode_front){
	case SCE_GXM_POLYGON_MODE_TRIANGLE_LINE:
		if (pol_offset_line) sceGxmSetFrontDepthBias(gxm_context, (int)pol_factor, (int)pol_units);
		else sceGxmSetFrontDepthBias(gxm_context, 0, 0);
		break;
	case SCE_GXM_POLYGON_MODE_TRIANGLE_POINT:
		if (pol_offset_point) sceGxmSetFrontDepthBias(gxm_context, (int)pol_factor, (int)pol_units);
		else sceGxmSetFrontDepthBias(gxm_context, 0, 0);
		break;
	case SCE_GXM_POLYGON_MODE_TRIANGLE_FILL:
		if (pol_offset_fill) sceGxmSetFrontDepthBias(gxm_context, (int)pol_factor, (int)pol_units);
		else sceGxmSetFrontDepthBias(gxm_context, 0, 0);
		break;
	}
	switch (polygon_mode_back){
	case SCE_GXM_POLYGON_MODE_TRIANGLE_LINE:
		if (pol_offset_line) sceGxmSetBackDepthBias(gxm_context, (int)pol_factor, (int)pol_units);
		else sceGxmSetBackDepthBias(gxm_context, 0, 0);
		break;
	case SCE_GXM_POLYGON_MODE_TRIANGLE_POINT:
		if (pol_offset_point) sceGxmSetBackDepthBias(gxm_context, (int)pol_factor, (int)pol_units);
		else sceGxmSetBackDepthBias(gxm_context, 0, 0);
		break;
	case SCE_GXM_POLYGON_MODE_TRIANGLE_FILL:
		if (pol_offset_fill) sceGxmSetBackDepthBias(gxm_context, (int)pol_factor, (int)pol_units);
		else sceGxmSetBackDepthBias(gxm_context, 0, 0);
		break;
	}	
}

static void change_cull_mode(){
	
	// Setting proper cull mode in sceGxm depending to current openGL machine state
	if (cull_face_state){
		if ((gl_front_face == GL_CW) && (gl_cull_mode == GL_BACK)) sceGxmSetCullMode(gxm_context, SCE_GXM_CULL_CCW);
		else if ((gl_front_face == GL_CCW) && (gl_cull_mode == GL_BACK)) sceGxmSetCullMode(gxm_context, SCE_GXM_CULL_CW);
		else if ((gl_front_face == GL_CCW) && (gl_cull_mode == GL_FRONT)) sceGxmSetCullMode(gxm_context, SCE_GXM_CULL_CCW);
		else if ((gl_front_face == GL_CW) && (gl_cull_mode == GL_FRONT)) sceGxmSetCullMode(gxm_context, SCE_GXM_CULL_CW);
		else if (gl_cull_mode == GL_FRONT_AND_BACK) no_polygons_mode = GL_TRUE;
	}else sceGxmSetCullMode(gxm_context, SCE_GXM_CULL_NONE);
	
}
 
/*
 * ------------------------------
 * - IMPLEMENTATION STARTS HERE -
 * ------------------------------
 */
 
void glPolygonMode(GLenum face,  GLenum mode){
	SceGxmPolygonMode new_mode;
	switch (mode){
		case GL_POINT:
			new_mode = SCE_GXM_POLYGON_MODE_TRIANGLE_POINT;
			break;
		case GL_LINE:
			new_mode = SCE_GXM_POLYGON_MODE_TRIANGLE_LINE;
			break;
		case GL_FILL:
			new_mode = SCE_GXM_POLYGON_MODE_TRIANGLE_FILL;
			break;
		default:
			error = GL_INVALID_ENUM;
			break;
	}
	switch (face){
		case GL_FRONT:
			polygon_mode_front = new_mode;
			gl_polygon_mode_front = mode;
			sceGxmSetFrontPolygonMode(gxm_context, new_mode);
			break;
		case GL_BACK:
			polygon_mode_back = new_mode;
			gl_polygon_mode_back = mode;
			sceGxmSetBackPolygonMode(gxm_context, new_mode);
			break;
		case GL_FRONT_AND_BACK:
			polygon_mode_front = polygon_mode_back = new_mode;
			gl_polygon_mode_front = gl_polygon_mode_back = mode;
			sceGxmSetFrontPolygonMode(gxm_context, new_mode);
			sceGxmSetBackPolygonMode(gxm_context, new_mode);
			break;
		default:
			error = GL_INVALID_ENUM;
			return;
	}
	update_polygon_offset();
}

void glPolygonOffset(GLfloat factor, GLfloat units){
	pol_factor = factor;
	pol_units = units;
	update_polygon_offset();
}
 
void glCullFace(GLenum mode){
	gl_cull_mode = mode;
	if (cull_face_state) change_cull_mode();
}

void glFrontFace(GLenum mode){
	gl_front_face = mode;
	if (cull_face_state) change_cull_mode();
}

void glViewport(GLint x,  GLint y,  GLsizei width,  GLsizei height){
	if ((width < 0) || (height < 0)){
		error = GL_INVALID_VALUE;
		return;
	}
	x_scale = width>>1;
	x_port = x + x_scale;
	y_scale = -(height>>1);
	y_port = DISPLAY_HEIGHT - y + y_scale;
	sceGxmSetViewport(gxm_context, x_port, x_scale, y_port, y_scale, z_port, z_scale);
	gl_viewport.x = x;
	gl_viewport.y = y;
	gl_viewport.w = width;
	gl_viewport.h = height;
	viewport_mode = 1;
}

void glDepthRange(GLdouble nearVal, GLdouble farVal){
	z_port = (farVal + nearVal) / 2.0f;
	z_scale = (farVal - nearVal) / 2.0f;
	sceGxmSetViewport(gxm_context, x_port, x_scale, y_port, y_scale, z_port, z_scale);
	viewport_mode = 1;
}

void glDepthRangef(GLfloat nearVal, GLfloat farVal){
	z_port = (farVal + nearVal) / 2.0f;
	z_scale = (farVal - nearVal) / 2.0f;
	sceGxmSetViewport(gxm_context, x_port, x_scale, y_port, y_scale, z_port, z_scale);
	viewport_mode = 1;
}

void glEnable(GLenum cap){
	if (phase == MODEL_CREATION){
		error = GL_INVALID_OPERATION;
		return;
	}
	switch (cap){
	case GL_DEPTH_TEST:
		depth_test_state = GL_TRUE;
		change_depth_func();
		break;
	case GL_STENCIL_TEST:
		change_stencil_settings();
		stencil_test_state = GL_TRUE;
		break;
	case GL_BLEND:
		if (!blend_state) change_blend_factor();
		blend_state = GL_TRUE;
		break;
	case GL_SCISSOR_TEST:
		scissor_test_state = GL_TRUE;
		update_scissor_test();
		break;
	case GL_CULL_FACE:
		cull_face_state = GL_TRUE;
		change_cull_mode();
		break;
	case GL_POLYGON_OFFSET_FILL:
		pol_offset_fill = GL_TRUE;
		update_polygon_offset();
		break;
	case GL_POLYGON_OFFSET_LINE:
		pol_offset_line = GL_TRUE;
		update_polygon_offset();
		break;
	case GL_POLYGON_OFFSET_POINT:
		pol_offset_point = GL_TRUE;
		update_polygon_offset();
		break;
	case GL_TEXTURE_2D:
		texture_units[server_texture_unit].enabled = GL_TRUE;
		break;
	case GL_ALPHA_TEST:
		alpha_test_state = GL_TRUE;
		update_alpha_test_settings();
		break;
	default:
		error = GL_INVALID_ENUM;
		break;
	}
}

void glDisable(GLenum cap){
	if (phase == MODEL_CREATION){
		error = GL_INVALID_OPERATION;
		return;
	}
	switch (cap){
	case GL_DEPTH_TEST:
		depth_test_state = GL_FALSE;
		change_depth_func();
		break;
	case GL_STENCIL_TEST:
		sceGxmSetFrontStencilFunc(gxm_context,
			SCE_GXM_STENCIL_FUNC_ALWAYS,
			SCE_GXM_STENCIL_OP_KEEP,
			SCE_GXM_STENCIL_OP_KEEP,
			SCE_GXM_STENCIL_OP_KEEP,
			0, 0);
		sceGxmSetBackStencilFunc(gxm_context,
			SCE_GXM_STENCIL_FUNC_ALWAYS,
			SCE_GXM_STENCIL_OP_KEEP,
			SCE_GXM_STENCIL_OP_KEEP,
			SCE_GXM_STENCIL_OP_KEEP,
			0, 0);
		stencil_test_state = GL_FALSE;
		break;
	case GL_BLEND:
		if (blend_state) disable_blend();
		blend_state = GL_FALSE;
		break;
	case GL_SCISSOR_TEST:
		scissor_test_state = GL_FALSE;
		update_scissor_test();
		break;
	case GL_CULL_FACE:
		cull_face_state = GL_FALSE;
		change_cull_mode();
		break;
	case GL_POLYGON_OFFSET_FILL:
		pol_offset_fill = GL_FALSE;
		update_polygon_offset();
		break;
	case GL_POLYGON_OFFSET_LINE:
		pol_offset_line = GL_FALSE;
		update_polygon_offset();
		break;
	case GL_POLYGON_OFFSET_POINT:
		pol_offset_point = GL_FALSE;
		update_polygon_offset();
		break;
	case GL_TEXTURE_2D:
		texture_units[server_texture_unit].enabled = GL_FALSE;
		break;
	case GL_ALPHA_TEST:
		alpha_test_state = GL_FALSE;
		update_alpha_test_settings();
		break;
	default:
		error = GL_INVALID_ENUM;
		break;
	}
}

void glClear(GLbitfield mask){
	GLenum orig_depth_test = depth_test_state;
	if ((mask & GL_COLOR_BUFFER_BIT) == GL_COLOR_BUFFER_BIT){
		invalidate_depth_test();
		change_depth_write(SCE_GXM_DEPTH_WRITE_DISABLED);
		sceGxmSetFrontPolygonMode(gxm_context, SCE_GXM_POLYGON_MODE_TRIANGLE_FILL);
		sceGxmSetBackPolygonMode(gxm_context, SCE_GXM_POLYGON_MODE_TRIANGLE_FILL);
		sceGxmSetVertexProgram(gxm_context, clear_vertex_program_patched);
		sceGxmSetFragmentProgram(gxm_context, clear_fragment_program_patched);
		void *color_buffer;
		sceGxmReserveFragmentDefaultUniformBuffer(gxm_context, &color_buffer);
		sceGxmSetUniformDataF(color_buffer, clear_color, 0, 4, &clear_rgba_val.r);
		sceGxmSetVertexStream(gxm_context, 0, clear_vertices);
		sceGxmDraw(gxm_context, SCE_GXM_PRIMITIVE_TRIANGLE_FAN, SCE_GXM_INDEX_FORMAT_U16, depth_clear_indices, 4);
		validate_depth_test();
		change_depth_write((depth_mask_state && orig_depth_test) ? SCE_GXM_DEPTH_WRITE_ENABLED : SCE_GXM_DEPTH_WRITE_DISABLED);
		sceGxmSetFrontPolygonMode(gxm_context, polygon_mode_front);
		sceGxmSetBackPolygonMode(gxm_context, polygon_mode_back);
		drawing = 1;
	}
	if ((mask & GL_DEPTH_BUFFER_BIT) == GL_DEPTH_BUFFER_BIT){
		invalidate_depth_test();
		change_depth_write(SCE_GXM_DEPTH_WRITE_ENABLED);
		depth_vertices[0].position.z = depth_value;
		depth_vertices[1].position.z = depth_value;
		depth_vertices[2].position.z = depth_value;
		depth_vertices[3].position.z = depth_value;
		sceGxmSetVertexProgram(gxm_context, disable_color_buffer_vertex_program_patched);
		sceGxmSetFragmentProgram(gxm_context, disable_color_buffer_fragment_program_patched);
		sceGxmSetVertexStream(gxm_context, 0, depth_vertices);
		sceGxmDraw(gxm_context, SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, SCE_GXM_INDEX_FORMAT_U16, depth_clear_indices, 4);
		validate_depth_test();
		change_depth_write((depth_mask_state && orig_depth_test) ? SCE_GXM_DEPTH_WRITE_ENABLED : SCE_GXM_DEPTH_WRITE_DISABLED);
	}
	if ((mask & GL_STENCIL_BUFFER_BIT) == GL_STENCIL_BUFFER_BIT){
		invalidate_depth_test();
		change_depth_write(SCE_GXM_DEPTH_WRITE_DISABLED);
		sceGxmSetVertexProgram(gxm_context, disable_color_buffer_vertex_program_patched);
		sceGxmSetFragmentProgram(gxm_context, disable_color_buffer_fragment_program_patched);
		sceGxmSetFrontStencilFunc(gxm_context,
			SCE_GXM_STENCIL_FUNC_NEVER,
			SCE_GXM_STENCIL_OP_REPLACE,
			SCE_GXM_STENCIL_OP_REPLACE,
			SCE_GXM_STENCIL_OP_REPLACE,
			0, stencil_value);
		sceGxmSetBackStencilFunc(gxm_context,
			SCE_GXM_STENCIL_FUNC_NEVER,
			SCE_GXM_STENCIL_OP_REPLACE,
			SCE_GXM_STENCIL_OP_REPLACE,
			SCE_GXM_STENCIL_OP_REPLACE,
			0, stencil_value);
		sceGxmSetVertexStream(gxm_context, 0, clear_vertices);
		sceGxmDraw(gxm_context, SCE_GXM_PRIMITIVE_TRIANGLE_FAN, SCE_GXM_INDEX_FORMAT_U16, depth_clear_indices, 4);
		validate_depth_test();
		change_depth_write((depth_mask_state && orig_depth_test) ? SCE_GXM_DEPTH_WRITE_ENABLED : SCE_GXM_DEPTH_WRITE_DISABLED);
		change_stencil_settings();
	}
}

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha){
	clear_rgba_val.r = red;
	clear_rgba_val.g = green;
	clear_rgba_val.b = blue;
	clear_rgba_val.a = alpha;
}

void glReadPixels(GLint x,  GLint y,  GLsizei width,  GLsizei height,  GLenum format,  GLenum type,  GLvoid * data){
	SceDisplayFrameBuf pParam;
	pParam.size = sizeof(SceDisplayFrameBuf);
	sceDisplayGetFrameBuf(&pParam, SCE_DISPLAY_SETBUF_NEXTFRAME);
	y = DISPLAY_HEIGHT - (height + y);
	int i,j;
	uint8_t* out8 = (uint8_t*)data;
	uint8_t* in8 = (uint8_t*)pParam.base;
	uint32_t* out32 = (uint32_t*)data;
	uint32_t* in32 = (uint32_t*)pParam.base;
	switch (format){
		case GL_RGBA:
			switch (type){
				case GL_UNSIGNED_BYTE:
					in32 += (x + y * pParam.pitch);
					for (i = 0; i < height; i++){
						for (j = 0; j < width; j++){
							out32[(height-(i+1))*width+j] = in32[j];
						}
						in32 += pParam.pitch;
					}
					break;
				default:
					error = GL_INVALID_ENUM;
					break;
			}
			break;
		case GL_RGB:
			switch (type){
				case GL_UNSIGNED_BYTE:
					in8 += (x * 4 + y * pParam.pitch * 4);
					for (i = 0; i < height; i++){
						for (j = 0; j < width; j++){
							out8[((height-(i+1))*width+j)*3] = in8[j*4];
							out8[((height-(i+1))*width+j)*3+1] = in8[j*4+1];
							out8[((height-(i+1))*width+j)*3+2] = in8[j*4+2];
						}
						in8 += pParam.pitch * 4;
					}
					break;
				default:
					error = GL_INVALID_ENUM;
					break;
			}
			break;
		default:
			error = GL_INVALID_ENUM;
			break;
	}
}

void glLineWidth(GLfloat width){
	
#ifndef SKIP_ERROR_HANDLING
	// Error handling
	if (width <= 0){
		error = GL_INVALID_VALUE;
		return;
	}
#endif

	// Changing line and point width as requested
	sceGxmSetFrontPointLineWidth(gxm_context, width);
	sceGxmSetBackPointLineWidth(gxm_context, width);
	
}

void glPointSize(GLfloat size){
	
#ifndef SKIP_ERROR_HANDLING
	// Error handling
	if (size <= 0){
		error = GL_INVALID_VALUE;
		return;
	}
#endif

	// Changing line and point width as requested
	sceGxmSetFrontPointLineWidth(gxm_context, size);
	sceGxmSetBackPointLineWidth(gxm_context, size);
	
}
