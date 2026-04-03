#pragma once

void mouse_pipeline_begin();
void mouse_pipeline_reset();
void mouse_pipeline_add_delta(double dx, double dy);
void mouse_pipeline_flush();
