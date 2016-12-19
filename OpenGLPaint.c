/*Xiao Li Wu*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#elif __linux__
#include <GL/glut.h>
#endif
#include <math.h>

const char *colors[] = {" ", "Red", "Blue", "Orange", "Pink", "Turquoise", "Black", "Green", "Purple"};

#define X_RESOLUTION 640
#define Y_RESOLUTION 480

#define RECTANGLE_FILLED 10
#define RECTANGLE_OUTLINE 20
#define ELLIPSE_FILLED 30
#define ELLIPSE_OUTLINE 40
#define LINE 50
#define BEZIER 60

#define RED 1
#define BLUE 2
#define ORANGE 3
#define PINK 4
#define TURQOISE 5
#define BLACK 6
#define GREEN 7
#define PURPLE 8

#define FILLED 0
#define OUTLINE 1

float camera_position_x = 0, camera_position_y = 0, camera_position_z = 0;
float line_of_sight_x = 0, line_of_sight_y = 0, line_of_sight_z = 0;

typedef struct shape_node
{
    int index;
    int shape;
    int color;
    int x_shape;
    int y_shape;
    struct shape_node *next;
}shape_node;

shape_node *shape_head = NULL;
shape_node *shape_current = NULL;

shape_node *drawn_head = NULL;
shape_node *drawn_current = NULL;

//check how many clicks based on selection.
int clicks_flag = -1;

int shape_select;
int color_select;

//counter index for LL
int counter = 0;
int drawn_counter = 0;

void reshape (int w, int h)
{
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (0.0, X_RESOLUTION, Y_RESOLUTION, 0, -1.0, 1.0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
}


int *draw_line(GLfloat red, GLfloat green, GLfloat blue, float x_start, float y_start, float x_end, float y_end)
{
    glLineWidth(1.0);
    glColor3f(red, green, blue);
    glBegin(GL_LINES);
    glVertex3f(x_start, y_start, 0.0);
    glVertex3f(x_end, y_end, 0.0);
    glEnd();
    
    return 0;
}

int *draw_rectangle(GLfloat red, GLfloat green, GLfloat blue, float x_start, float y_start, float x_end, float y_end, int filled_or_outline)
{
    
    if(filled_or_outline == FILLED)
    {
        glColor3f(red, green, blue);
        glBegin(GL_POLYGON);
        glVertex3f(x_start, y_start, 0.0);
        glVertex3f(x_end, y_start, 0.0);
        glVertex3f(x_end, y_end, 0.0);
        glVertex3f(x_start, y_end, 0.0);
        glEnd();
    }
    else
    {
        glColor3f(red, green, blue);
        glBegin(GL_LINE_LOOP);
        glVertex3f(x_start, y_start, 0.0);
        glVertex3f(x_end, y_start, 0.0);
        glVertex3f(x_end, y_end, 0.0);
        glVertex3f(x_start, y_end, 0.0);
        glEnd();
    }
    
    return 0;
}

int *draw_ellipse(GLfloat red, GLfloat green, GLfloat blue, float xCenter, float yCenter, float rx, float ry, int filled_or_outline)
{
    int i;
    const float DEG2RAD = 3.14159/180.0;
    glColor3f(red, green, blue);
    
    float radiusX;
    float radiusY;
    
    float cx = yCenter;
    float cy = rx;
    
   
        radiusX = sqrt(((cx - xCenter)*(cx - xCenter)) + ((cy - yCenter)*(cy - yCenter)));
        
        //major_axis
        radiusY = sqrt(((rx - xCenter)*(rx - xCenter)) + ((ry - yCenter)*(ry - yCenter)));
    
    if(filled_or_outline == FILLED)
    {
        glBegin(GL_POLYGON);
        
        for(i=0; i<360; i++) {
            float rad = i*DEG2RAD;
            glVertex3f(xCenter + radiusX*cos(rad),
                       yCenter + radiusY*sin(rad), 0.0);
        }
        
        glEnd();
    }
    else
    {
        glBegin(GL_LINE_LOOP);
        
        for(i=0; i<360; i++) {
            float rad = i*DEG2RAD;
            glVertex3f(xCenter + radiusX*cos(rad),
                       yCenter + radiusY*sin(rad), 0.0);
        }
        
        glEnd();
    }
    
    return 0;
}

int *draw_bezier(GLfloat red, GLfloat green, GLfloat blue, float xA, float yA, float xB, float yB, float xC, float yC, float xD, float yD)
{
    float a = 1.0;
    float b = 1.0 - a;
    
    float X, Y;
    
    int i;
    
    glLineWidth(1.0);
    glColor3f(red, green, blue);
    glBegin(GL_LINE_STRIP);
    
    for(i = 0; i< 20; i++)
    {
        X = xA*a*a*a + xB*3*a*a*b + xC*3*a*b*b + xD*b*b*b;
        Y = yA*a*a*a + yB*3*a*a*b + yC*3*a*b*b + yD*b*b*b;
        
        glVertex3f(X,Y,0.0);
        
        a -= 0.05;
        b = 1.0 - a;
    }
    
    glEnd();
    return 0;
}

int draw(shape_node *shapes)
{
    int shape_name = shapes->shape;
    
    float x_start;
    float y_start;
    float x_end;
    float y_end;
    
    float bx_start;
    float by_start;
    float bx_second;
    float by_second;
    float bx_third;
    float by_third;
    float bx_end;
    float by_end;
    
    if(shape_name == BEZIER)
    {
        shape_node *start_coord = shapes;
        shape_node *second_coord = shapes->next;
        shape_node *third_coord;
        shape_node *end_coord;
        
        if(second_coord)
        {
            int second_index = second_coord->index;
            if(second_index < (drawn_counter - 1))
            {
                if(second_coord->shape != start_coord->shape)
                {
                    return 0;
                }
                else
                {
                    third_coord = second_coord->next;
                    if(third_coord)
                    {
                        int third_index = third_coord->index;
                        if(third_index < (drawn_counter - 1))
                        {
                            if(third_coord->shape != second_coord->shape)
                            {
                                return 0;
                            }
                            else
                            {
                                end_coord = third_coord->next;
                                if(end_coord)
                                {
                                    if(end_coord->shape != third_coord->shape)
                                    {
                                        return 0;
                                    }
                                }
                                else
                                {
                                    return 0;
                                }
                            }
                        }
                        else
                        {
                            return 0;
                        }
                    }//third_coord
                    else
                    {
                        return 0;
                    }
                }
            }//second index
            else
            {
                return 0;
            }
        }//second_coord
        else
        {
            return 0;
        }
        
        bx_start = (float)start_coord->x_shape;
        by_start = (float)start_coord->y_shape;
        bx_second = (float)second_coord->x_shape;
        by_second = (float)second_coord->y_shape;
        bx_third = (float)third_coord->x_shape;
        by_third = (float)third_coord->y_shape;
        bx_end = (float)end_coord->x_shape;
        by_end = (float)end_coord->y_shape;
    }
    else
    {
        shape_node *start_coord = shapes;
        shape_node *end_coord = shapes->next;
        if(end_coord)
        {
            if(end_coord->shape != start_coord->shape)
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
        
        x_start = (float)start_coord->x_shape;
        y_start = (float)start_coord->y_shape;
        x_end = (float)end_coord->x_shape;
        y_end = (float)end_coord->y_shape;
    }
    
    GLfloat red = 0.0;
    GLfloat green = 0.0;
    GLfloat blue = 0.0;
    
    switch (shapes->color) {
        case RED:
            red = 1.0;
            green = 0.0;
            blue = 0.0;
            break;
        case BLUE:
            red = 0.0;
            green = 0.0;
            blue = 1.0;
            break;
        case ORANGE:
            red = 1.0;
            green = 0.5;
            blue = 0.0;
            break;
        case PINK:
            red = 1.00;
            green = 0.43;
            blue = 0.78;
            break;
        case TURQOISE:
            red = 0.678431;
            green = 0.917647;
            blue = 0.917647;
            break;
        case BLACK:
            red = 0.0;
            green = 0.0;
            blue = 0.0;
            break;
        case GREEN:
            red = 0.0;
            green = 1.0;
            blue = 0.0;
            break;
        case PURPLE:
            red = 0.73;
            green = 0.16;
            blue = 0.96;
            break;
        default:
            break;
    }
    
    switch(shape_name)
    {
        case RECTANGLE_FILLED:
            draw_rectangle(red, green, blue, x_start, y_start, x_end, y_end, FILLED);
            break;
        case RECTANGLE_OUTLINE:
            draw_rectangle(red, green, blue, x_start, y_start, x_end, y_end, OUTLINE);
            break;
        case ELLIPSE_FILLED:
            draw_ellipse(red, green, blue, x_start, y_start, x_end, y_end, FILLED);
            break;
        case ELLIPSE_OUTLINE:
            draw_ellipse(red, green, blue, x_start, y_start, x_end, y_end, OUTLINE);
            break;
        case LINE:
            draw_line(red, green, blue, x_start, y_start, x_end, y_end);
            break;
        case BEZIER:
            //printf("bezier coordinates: %f %f %f %f %f %f %f %f \n", bx_start, by_start, bx_second, by_second, bx_third, by_third, bx_end, by_end);
            draw_bezier(red, green, blue, bx_start, by_start, bx_second, by_second, bx_third,by_third, bx_end, by_end);
            break;
        default:
            break;
    }
    
    
    return 0;
}

void loop_draw(shape_node *shp_node)
{
    draw(shp_node);
    //I finished drawing bezier
    int shape_drawn_name = shp_node->shape;
    
    if(shape_drawn_name == BEZIER)
    {
        shape_node *next_shape = shp_node->next->next->next;
        if(next_shape)
        {
            int next_index = next_shape->index;
            if(next_index < (drawn_counter - 1))
            {
                loop_draw(next_shape->next);
            }
        }
    }
    else
    {
        if(shp_node->next)
        {
            int next_index = shp_node->next->index;
            if(next_index < (drawn_counter - 1))
            {
                loop_draw(shp_node->next->next);
            }
        }
    }
}

void renderCanvas (void)
{

    glClearColor (1.0, 1.0, 1.0, 1.0);
    glClear (GL_COLOR_BUFFER_BIT);
    glLoadIdentity ();
    
    //draw right here give a linkedlist
    if(clicks_flag == 0)
    {
        if(shape_head)
        {
            //printf("about to draw \n");
            drawn_head = shape_head;
            drawn_current = shape_current;
            drawn_counter = counter;
        }
        
        clicks_flag = -1;
        //to say it has been drawn because it gets segmentation fault from 1 click
    }
    
    if(drawn_head)
    {
        //printf("clicks flag in render canvas %d \n", clicks_flag);
        loop_draw(drawn_head);
    }
    
    glPopMatrix();
    glFlush();
    glutSwapBuffers();
    glutPostRedisplay();
}


void insert_drawing(int index_ll, int shape_ll, int x_ll, int y_ll)
{
    shape_node *temp_link = (shape_node *)malloc(sizeof(shape_node));
    temp_link->index = counter;
    temp_link->shape = shape_select;
    temp_link->color = color_select;
    temp_link->x_shape = x_ll;
    temp_link->y_shape = y_ll;
    
    if(!shape_head) //if shape head is NULL
    {
        shape_head = temp_link;
        shape_current = temp_link;
    }
    else
    {
        shape_current->next = temp_link;
        shape_current = temp_link;
    }
    
    counter++;
}


int *shape_delete(int index_ll)
{
    shape_node *temp_shape = shape_head;
    
    while(temp_shape->index != index_ll)
    {
        if(temp_shape->next)
        {
            temp_shape = temp_shape->next;
        }
        else
        {
            printf("shape_delete is out of bounds");
            exit(0);
        }
    }
    
    //printf("temp_shape index: %d \n", temp_shape->index);
    
    shape_current = temp_shape;
    shape_current->next = NULL;
    counter++; //put the counter back to what the next link should have
    return 0;
}

void menu (int value)
{
    int i;
    int form = 0;
    for(i = 1; i < 9; i++)
    {
        form = value - i;
        //printf("form: %d \n", form);
        switch (form)
        {
            case RECTANGLE_FILLED:
                //printf("rectangle -> filled -> %s \n", colors[i]);
                //make rectangle filled color method and add it to the array and
                if(shape_select == BEZIER && clicks_flag > 0 && clicks_flag < 4)
                {
                    int delete_index = 5-clicks_flag; //4 for the shape clicks and 1 for the counter increment = 5
                    counter = counter - delete_index;
                    shape_delete(counter);
                    
                    clicks_flag = 2;
                }
                else if(clicks_flag > 0 && clicks_flag < 2)
                {
                    //if clicks_flag > 0 and is not 2 or 4 (meaning they didn't click anything but selected then delete
                    
                    int delete_index = 3-clicks_flag;
                    counter = counter - delete_index;
                    shape_delete(counter);
                    
                    clicks_flag = 2;
                }
                else
                {
                    clicks_flag = 2;
                }
                shape_select = RECTANGLE_FILLED;
                color_select = i;
                //in case user changes mind: clicks_flag is not zero nullify the number of clicks_flag LL
                break;
            case RECTANGLE_OUTLINE:
                //printf("rectangle -> outline -> %s \n", colors[i]);
                if(shape_select == BEZIER && clicks_flag > 0 && clicks_flag < 4)
                {
                    int delete_index = 5-clicks_flag;
                    counter = counter - delete_index;
                    shape_delete(counter);
                    
                    clicks_flag = 2;
                }
                else if(clicks_flag > 0 && clicks_flag < 2)
                {
                    //if clicks_flag > 0 and is not 2 or 4 (meaning they didn't click anything but selected then delete
                    int delete_index = 3-clicks_flag;
                    counter = counter - delete_index;
                    shape_delete(counter);
                    
                    clicks_flag = 2;
                }
                else
                {
                    clicks_flag = 2;
                }
                shape_select = RECTANGLE_OUTLINE;
                color_select = i;
                break;
            case ELLIPSE_FILLED:
                //printf("ellipse -> filled -> %s \n", colors[i]);
                if(shape_select == BEZIER && clicks_flag > 0 && clicks_flag < 4)
                {
                    int delete_index = 5-clicks_flag;
                    counter = counter - delete_index;
                    shape_delete(counter);
                    
                    clicks_flag = 2;
                }
                else if(clicks_flag > 0 && clicks_flag < 2)
                {
                    //if clicks_flag > 0 and is not 2 or 4 (meaning they didn't click anything but selected then delete
                    int delete_index = 3-clicks_flag;
                    counter = counter - delete_index;
                    shape_delete(counter);
                    
                    clicks_flag = 2;
                }
                else
                {
                    clicks_flag = 2;
                }
                shape_select = ELLIPSE_FILLED;
                color_select = i;
                break;
            case ELLIPSE_OUTLINE:
                //printf("ellipse -> outline -> %s \n", colors[i]);
                if(shape_select == BEZIER && clicks_flag > 0 && clicks_flag < 4)
                {
                    int delete_index = 5-clicks_flag;
                    counter = counter - delete_index;
                    shape_delete(counter);
                    
                    clicks_flag = 2;
                }
                else if(clicks_flag > 0 && clicks_flag < 2)
                {
                    //if clicks_flag > 0 and is not 2 or 4 (meaning they didn't click anything but selected then delete
                    int delete_index = 3-clicks_flag;
                    counter = counter - delete_index;
                    shape_delete(counter);
                    
                    clicks_flag = 2;
                }
                else
                {
                    clicks_flag = 2;
                }
                shape_select = ELLIPSE_OUTLINE;
                color_select = i;
                break;
            case LINE:
                //printf("line -> %s \n", colors[i]);
                if(shape_select == BEZIER && clicks_flag > 0 && clicks_flag < 4)
                {
                    int delete_index = 5-clicks_flag;
                    counter = counter - delete_index;
                    shape_delete(counter);
                    
                    clicks_flag = 2;
                }
                else if(clicks_flag > 0 && clicks_flag < 2)
                {
                    //if clicks_flag > 0 and is not 2 or 4 (meaning they didn't click anything but selected then delete
                    int delete_index = 3-clicks_flag;
                    counter = counter - delete_index;
                    shape_delete(counter);
                    
                    clicks_flag = 2;
                }
                else
                {
                    clicks_flag = 2;
                }
                shape_select = LINE;
                color_select = i;
                break;
            case BEZIER:
                //printf("bezier -> %s \n", colors[i]);
                if(shape_select == BEZIER && clicks_flag > 0 && clicks_flag < 4)
                {
                    int delete_index = 5-clicks_flag;
                    counter = counter - delete_index;
                    shape_delete(counter);
                    //printf("LINE 592 \n");
                    clicks_flag = 4;
                }
                else if(clicks_flag > 0 && clicks_flag < 2)
                {
                    //if clicks_flag > 0 and is not 2 or 4 (meaning they didn't click anything but selected then delete
                    int delete_index = 3-clicks_flag;
                    counter = counter - delete_index;
                    shape_delete(counter);
                    //printf("LINE 601 \n");
                    clicks_flag = 4;
                }
                else
                {
                    clicks_flag = 4;
                }
                shape_select = BEZIER;
                color_select = i;
                break;
            default:
                break;
        }
    }
    glutPostRedisplay();
}


void createPopupMenus()
{
    int rectangle_filled_menu, rectangle_outline_menu, ellipse_filled_menu, ellipse_outline_menu,
    rectangle_menu, ellipse_menu, line_menu, bezier_menu;
    
    int i;
    
    //rectangle filled menu
    rectangle_filled_menu = glutCreateMenu (menu);
    
    for( i = 1; i < 9; i++)
    {
        glutAddMenuEntry(colors[i], RECTANGLE_FILLED+i);
    }//11, 12, 13, 14, 15, 16, 17, 18
    
    //rectangle outline menu
    rectangle_outline_menu = glutCreateMenu(menu);
    
    for(i = 1; i < 9; i++)
    {
        glutAddMenuEntry(colors[i], RECTANGLE_OUTLINE+i);
    }//21, 22, 23, 24, 25, 26, 27, 28
    
    //ellipse filled menu
    ellipse_filled_menu = glutCreateMenu(menu);
    
    for(i = 1; i < 9; i++)
    {
        glutAddMenuEntry(colors[i], ELLIPSE_FILLED+i);
    }//31, 32, 33, 34, 35, 36, 37, 38
    
    //ellipse outline menu
    ellipse_outline_menu = glutCreateMenu(menu);
    
    for(i = 1; i < 9; i++)
    {
        glutAddMenuEntry(colors[i], ELLIPSE_OUTLINE+i);
    }//41, 42, 43, 44, 45, 46, 47, 48
    
    //line filled menu
    line_menu = glutCreateMenu(menu);
    
    for(i = 1; i < 9; i++)
    {
        glutAddMenuEntry(colors[i], LINE+i);
    }//51, 52, 53, ...
    
    
    //bezier filled menu
    bezier_menu = glutCreateMenu(menu);
    
    for(i = 1; i < 9; i++)
    {
        glutAddMenuEntry(colors[i], BEZIER+i);
    }
    
    rectangle_menu = glutCreateMenu (menu);
    glutAddSubMenu ("Filled", rectangle_filled_menu);
    glutAddSubMenu ("Outline", rectangle_outline_menu);
    
    ellipse_menu = glutCreateMenu(menu);
    glutAddSubMenu ("Filled", ellipse_filled_menu);
    glutAddSubMenu ("Outline", ellipse_outline_menu);
    
    glutCreateMenu (menu);
    glutAddSubMenu ("Add Rectangle", rectangle_menu);
    glutAddSubMenu("Add Ellipse", ellipse_menu);
    glutAddSubMenu("Add Line", line_menu);
    glutAddSubMenu("Add Bezier", bezier_menu);
    
    glutAttachMenu (GLUT_LEFT_BUTTON);
    
    
}


void OnMouseClick(int button, int state, int x, int y)
{
    if(button == GLUT_RIGHT_BUTTON && state == GLUT_UP && clicks_flag > 0)
    {
        insert_drawing(counter, shape_select, x, y);
        clicks_flag--;
    }
    
}

int main (int argc, char *argv[])
{
    int window;
    
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (X_RESOLUTION, Y_RESOLUTION);
    glutInitWindowPosition (0,0);
    window = glutCreateWindow ("HELLO MENU ART");
    
    //init menu
    createPopupMenus();
    
    //mouse click
    glutMouseFunc(OnMouseClick);
   
    glutDisplayFunc (renderCanvas);
    glutReshapeFunc (reshape);
    glutMainLoop ();  
}
