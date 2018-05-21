void display_camera(int cam_index) {

	// should optimize this dispaly function further to reduce the amount of floating-point operations.
	glm::mat4 Matrix_TIGER_tmp;
	glViewport(viewport[cam_index].x, viewport[cam_index].y, viewport[cam_index].w, viewport[cam_index].h);

		ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix[cam_index], glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glLineWidth(2.0f);
		//draw_axes(cam_index);
		glLineWidth(1.0f);
		draw_static_object(&(static_objects[OBJ_BUILDING]), 0, cam_index);
		draw_static_object(&(static_objects[OBJ_TABLE]), 0, cam_index);
		draw_static_object(&(static_objects[OBJ_TABLE]), 1, cam_index);
		draw_static_object(&(static_objects[OBJ_LIGHT]), 0, cam_index);
		draw_static_object(&(static_objects[OBJ_LIGHT]), 1, cam_index);
		draw_static_object(&(static_objects[OBJ_LIGHT]), 2, cam_index);
		draw_static_object(&(static_objects[OBJ_LIGHT]), 3, cam_index);
		draw_static_object(&(static_objects[OBJ_LIGHT]), 4, cam_index);

		draw_static_object(&(static_objects[OBJ_TEAPOT]), 0, cam_index);
		draw_static_object(&(static_objects[OBJ_NEW_CHAIR]), 0, cam_index);
		draw_static_object(&(static_objects[OBJ_FRAME]), 0, cam_index);
		draw_static_object(&(static_objects[OBJ_NEW_PICTURE]), 0, cam_index);
		draw_static_object(&(static_objects[OBJ_COW]), 0, cam_index);

		draw_animated_tiger(cam_index);
		draw_animated_teapot(cam_index);
	
}
void display_tiger(int cam_index)
{
	glm::mat4 Matrix_TIGER_tmp;

	glViewport(viewport[cam_index].x, viewport[cam_index].y, viewport[cam_index].w, viewport[cam_index].h);

	ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix[cam_index], glm::vec3(1.0f, 1.0f, 1.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	draw_animated_tiger(cam_index);
}