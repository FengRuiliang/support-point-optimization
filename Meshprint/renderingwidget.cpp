#pragma once
#include "renderingwidget.h"
#include <QKeyEvent>
#include <QColorDialog>
#include <QFileDialog>
#include <iostream>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QTextCodec>
#include <gl/GLU.h>
#include <gl/glut.h>
#include <algorithm>
#include <queue>
#include <utility>
#include "Library/ArcBall.h"
#include "QDebug"
#include "meshprint.h"
#include <fstream>
#include <QTime>


static GLfloat win, hei;
RenderingWidget::RenderingWidget(QWidget *parent, MainWindow* mainwindow)
	: QOpenGLWidget(parent), ptr_mainwindow_(mainwindow), eye_distance_(200),
	has_lighting_(true), is_draw_point_(false), is_draw_edge_(false), is_draw_face_(true)
{
	ptr_arcball_ = new CArcBall(width(), height());
	is_load_texture_ = false;
	is_draw_axes_ = false;
	is_draw_texture_ = (false);
	eye_goal_[0] = eye_goal_[1] = eye_goal_[2] = 0.0;
	eye_direction_[0] = eye_direction_[1] = 0.0;
	eye_direction_[2] = 1.0;
}

RenderingWidget::~RenderingWidget()
{

}

void RenderingWidget::initializeGL()
{
	glClearColor(0.78, 0.78, 0.78, 0.0);
	glClearColor(1, 1, 1, 0.0);
	glShadeModel(GL_SMOOTH);
	//glShadeModel(GL_FLAT);

	glEnable(GL_DOUBLEBUFFER);
	// 	glEnable(GL_POINT_SMOOTH);
	// 	glEnable(GL_LINE_SMOOTH);
	//	glEnable(GL_POLYGON_SMOOTH);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1);

	SetLight();
}

void RenderingWidget::resizeGL(int w, int h)
{
	h = (h == 0) ? 1 : h;
	win = w;
	hei = h;
	ptr_arcball_->reSetBound(w, h);
	//glViewport(0, 0, w, h);
	glViewport(0, 0, (GLfloat)eye_distance_*win, (GLfloat)eye_distance_*hei);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glOrtho(-eye_distance_, eye_distance_, -eye_distance_ * (GLfloat)h / (GLfloat)w, eye_distance_ * (GLfloat)h / (GLfloat)w, -200.0, 200.0);
	else
		glOrtho(-eye_distance_*(GLfloat)w / (GLfloat)h, eye_distance_*(GLfloat)w / (GLfloat)h, -eye_distance_, eye_distance_, -200.0, 200.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void RenderingWidget::paintGL()
{
	glShadeModel(GL_SMOOTH);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (has_lighting_)
	{
		SetLight();
	}
	else
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (win <= hei)
		glOrtho(-eye_distance_ + eye_goal_[0], eye_distance_ + eye_goal_[0],
			-eye_distance_ * (GLfloat)hei / (GLfloat)win + eye_goal_[1], eye_distance_ * (GLfloat)hei / (GLfloat)win + eye_goal_[1],
			-200.0, 200.0);
	else
		glOrtho(-eye_distance_ * (GLfloat)win / (GLfloat)hei + eye_goal_[0], eye_distance_ * (GLfloat)win / (GLfloat)hei + eye_goal_[0],
			-eye_distance_ + eye_goal_[1], eye_distance_ + eye_goal_[1], -200.0, 200.0);


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
// 	float mId[4][4];
// 		for (int i = 0; i < 4; i++)
// 		{
// 			for (int j = 0; j < 4; j++)
// 			{
// 				if (i == j)
// 					mId[i][j] = 1.f;
// 				else	mId[i][j] = 0.f;
// 			}
// 		}
// 		 	//mId[1][1] = 0;
// 		 	//mId[2][2] = 0;
// 		 	//mId[2][1] = 1;
// 		 //	mId[1][2] = -1;
// 		mId[1][1] = -1;
// 		mId[2][2] = -1;
// 	glMultMatrixf((float*)mId);
	glMultMatrixf(ptr_arcball_->GetBallMatrix());

	Render();

}

void RenderingWidget::timerEvent(QTimerEvent * e)
{
	update();
}

void RenderingWidget::mousePressEvent(QMouseEvent *e)
{

	switch (e->button())
	{
	case Qt::LeftButton:
	{
		makeCurrent();
		ptr_arcball_->MouseDown(e->pos());
		update();
	}
	break;
	case Qt::MidButton:
		current_position_ = e->pos();
		break;
	case  Qt::RightButton:
	{
		makeCurrent();
		break;
	}
	default:
		break;
	}
}
void RenderingWidget::mouseMoveEvent(QMouseEvent *e)
{
	switch (e->buttons())
	{
		setCursor(Qt::ClosedHandCursor);

	case Qt::LeftButton:
		ptr_arcball_->MouseMove(e->pos());
		break;

	case Qt::MidButton:
		eye_goal_[0] -= GLfloat(e->x() - current_position_.x()) / GLfloat(width());
		eye_goal_[1] += GLfloat(e->y() - current_position_.y()) / GLfloat(height());
		current_position_ = e->pos();
		break;
	default:
		break;
	}
	update();
}
void RenderingWidget::mouseReleaseEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:

		ptr_arcball_->MouseUp(e->pos());
		setCursor(Qt::ArrowCursor);

		ptr_arcball_->MouseUp(e->pos());
		setCursor(Qt::ArrowCursor);

		break;
	case Qt::RightButton:
		break;
	default:
		break;
	}
}
void RenderingWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	default:
		break;
	}
	update();
}

void RenderingWidget::wheelEvent(QWheelEvent *e)
{

	eye_distance_ -= e->delta()/10;
	eye_distance_ = eye_distance_ < 0 ? 0 : eye_distance_;
	update();
}

void RenderingWidget::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_A:
		break;
	default:
		break;
	}
}

void RenderingWidget::keyReleaseEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_A:
		break;
	default:
		break;
	}
}

void RenderingWidget::Render()
{
	DrawAxes(is_draw_axes_);
	DrawPoints(is_draw_point_);
	DrawEdge(is_draw_edge_);
	DrawFace(is_draw_face_);
	DrawTexture(is_draw_texture_);
	DrawSlice(true);
	DrawDebug(true);
}

void RenderingWidget::SetLight()
{
	//return;
	static GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static GLfloat mat_shininess[] = { 50.0f };
	static GLfloat light_position0[] = { 0.0f, 10.0f, 0.5f, 0.0f };
	static GLfloat light_position1[] = { 0.0f, -10.0f, 0.5f, 0.0f };
	static GLfloat light_position2[] = { 0.0f, 0.0f, -0.5f, 0.0f };
	static GLfloat bright[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	static GLfloat dim_light[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	static GLfloat lmodel_ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_specular);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, bright);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, bright);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, bright);
	glLightfv(GL_LIGHT2, GL_POSITION, light_position2);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, bright);
	//glLightfv(GL_LIGHT1, GL_SPECULAR, white_light);
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
}

void RenderingWidget::SetBackground()
{
	QColor color = QColorDialog::getColor(Qt::white, this, tr("background color"));
	GLfloat r = (color.red()) / 255.0f;
	GLfloat g = (color.green()) / 255.0f;
	GLfloat b = (color.blue()) / 255.0f;
	GLfloat alpha = color.alpha() / 255.0f;
	makeCurrent();
	glClearColor(r, g, b, alpha);

	//updateGL();
	update();
}



void RenderingWidget::ReadMesh()
{
	
	QString filename = QFileDialog::
		getOpenFileName(this, tr("Read Mesh"),
			"Resources/models", tr("Meshes (*.obj *.stl)"));

	if (filename.isEmpty())
	{
		//emit(operatorInfo(QString("Read Mesh Failed!")));
		return;
	}
	//中文路径支持
	QTextCodec *code = QTextCodec::codecForName("gd18030");
	QTextCodec::setCodecForLocale(code);

	//mycut->clearcut();

	QByteArray byfilename = filename.toLocal8Bit();
	QFileInfo fileinfo = QFileInfo(filename);


	if (fileinfo.suffix() == "obj")
	{
		procesoor.read_obj_file(byfilename);
		procesoor.update_my_mesh();
	}
	else if (fileinfo.suffix() == "stl" || fileinfo.suffix() == "STL")
	{
	}
	update();
}

void RenderingWidget::WriteMesh()
{

	QString filename = QFileDialog::
		getSaveFileName(this, tr("Write Mesh"),
			"..", tr("Meshes (*.txt)"));
	if (filename.isEmpty())
		return;
	QByteArray byfilename = filename.toLocal8Bit();
}


void RenderingWidget::CheckDrawPoint()
{
	is_draw_point_ = !is_draw_point_;
	update();

}
void RenderingWidget::CheckDrawEdge()
{
	is_draw_edge_ = !is_draw_edge_;
	update();

}
void RenderingWidget::CheckDrawFace()
{
	is_draw_face_ = !is_draw_face_;
	update();

}
void RenderingWidget::CheckLight()
{
	has_lighting_ = !has_lighting_;
	update();

}
void RenderingWidget::CheckDrawTexture()
{
	is_draw_texture_ = !is_draw_texture_;
	if (is_draw_texture_)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);
	update();

}
void RenderingWidget::CheckDrawAxes()
{
	is_draw_axes_ = !is_draw_axes_;
	//updateGL();
	update();

}



void RenderingWidget::DrawAxes(bool bv)
{
	if (!bv)
		return;
	//x axis
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0.7, 0.0, 0.0);
	glEnd();
	glPushMatrix();
	glTranslatef(0.7, 0, 0);
	glRotatef(90, 0.0, 1.0, 0.0);
	//glutSolidCone(0.02, 0.06, 20, 10);
	glPopMatrix();

	//y axis
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0.0, 0.7, 0.0);
	glEnd();

	glPushMatrix();
	glTranslatef(0.0, 0.7, 0);
	glRotatef(90, -1.0, 0.0, 0.0);
	//glutSolidCone(0.02, 0.06, 20, 10);
	glPopMatrix();

	//z axis
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0.0, 0.0, 0.7);
	glEnd();
	glPushMatrix();
	glTranslatef(0.0, 0, 0.7);
	//glutSolidCone(0.02, 0.06, 20, 10);
	glPopMatrix();

	glColor3f(1.0, 1.0, 1.0);
}
void RenderingWidget::DrawPoints(bool bv)
{

}
void RenderingWidget::DrawEdge(bool bv)
{

}
void RenderingWidget::DrawFace(bool bv)
{
	if (!bv)
	{
		return;
	}
	bool found;
	Mesh& pm = procesoor.mesh;
	Mesh::Property_map<face_descriptor, K::Vector_3> fnormals;
	boost::tie(fnormals,found)=pm.property_map<face_descriptor, K::Vector_3>("f:normals");
	if (found)
	{
		glColor3ub(0, 170, 0);
		glBegin(GL_TRIANGLES);
		K::Point_3 p;
		for (Mesh::Face_iterator iter = pm.faces_begin(); iter != pm.faces_end(); iter++)
		{
			glNormal3f(fnormals[*iter].x(), fnormals[*iter].y(), fnormals[*iter].z());
			auto vsiter = pm.vertices_around_face(pm.halfedge(*iter));
			for (auto viter = vsiter.begin(); viter != vsiter.end(); viter++)
			{
				p = pm.point(*viter);
				glVertex3f(p.x(), p.y(), p.z());
			}
		}
		glEnd();
	}
	
}
void RenderingWidget::DrawTexture(bool bv)
{
	
}
void RenderingWidget::DrawSlice(bool bv)
{
	if (!bv)
	{
		return;
	}
	std::vector<Polylines>& ctours=procesoor.contours;
	glColor3ub(255, 0, 0);
	for (int i=0;i<ctours.size();i++)
	{
		if (i!=i_th_slice)
		{
			continue;
		}
		for (auto iterlines = ctours[i].begin(); iterlines != ctours[i].end(); iterlines++)
		{
			glBegin(GL_LINE_LOOP);
			for (auto iterpoints = (*iterlines).begin(); iterpoints != (*iterlines).end(); iterpoints++)
			{
				glVertex3f(iterpoints->x(), iterpoints->y(), iterpoints->z());
			}
			glEnd();
		}
	}
}
void RenderingWidget::DrawHatch(bool bv)
{
	
}

void RenderingWidget::DrawDebug(bool param1)
{

	glBegin(GL_LINES);
	glColor3ub(0, 170, 0);
	std::vector<std::vector<std::tuple<double, double, double, double>>> tupes = procesoor.support_region_voronoi_diagrams;
	for (int slice_id=0;slice_id<tupes.size();slice_id++)
	{
		if (slice_id!=i_th_slice)
		{
			continue;
		}
		for (auto iterS=tupes[slice_id].begin();iterS!=tupes[slice_id].end();iterS++)
		{
			GLfloat x1, y1,x2, y2;
			std::tie(x1, y1, x2, y2) = *iterS;
			glVertex3f(x1,y1,slice_id);
			glVertex3f(x2, y2, slice_id);
		}
	}
	glEnd();
}

void RenderingWidget::DoSliceAndHatch()
{
	procesoor.do_slice();
}
void RenderingWidget::AddSupportStructure()
{
	procesoor.add_support();
}
void RenderingWidget::SetSliceCheckId(int val)
{
	i_th_slice = val;
	update();
}