/*
 * gEDA - GNU Electronic Design Automation
 * This file is a part of gerbv.
 *
 *   Copyright (C) 2008 Julian Lamb
 *
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <math.h>

#include <glib/gstdio.h>
#include "gerber.h"
#include <export-rs274x.h>
#include "draw-gdk.h"

/* DEBUG printing.  #define DEBUG 1 in config.h to use this fcn. */
#define dprintf if(DEBUG) printf
#define round(x) floor(x+0.5)

void
export_rs274x_write_macro (FILE *fd, gerb_aperture_t *currentAperture,
			gint apertureNumber) {
	gerb_simplified_amacro_t *ls = currentAperture->simplified;

	/* write the macro portion first */
	fprintf(fd, "%%AMMACRO%d*\n",apertureNumber);
	while (ls != NULL) {
		if (ls->type == MACRO_CIRCLE) {
			fprintf(fd, "1,%d,%f,%f,%f*\n",(int) ls->parameter[CIRCLE_EXPOSURE],
				ls->parameter[CIRCLE_DIAMETER],ls->parameter[CIRCLE_CENTER_X],
				ls->parameter[CIRCLE_CENTER_Y]);
		}
		else if (ls->type == MACRO_OUTLINE) {
			int pointCounter;
			int numberOfPoints = (int) ls->parameter[OUTLINE_NUMBER_OF_POINTS];
			
			fprintf(fd, "4,%d,%d,\n",(int) ls->parameter[OUTLINE_EXPOSURE],
				numberOfPoints);
			
			for (pointCounter=0; pointCounter < numberOfPoints; pointCounter++) {
			    fprintf(fd, "%f,%f,",ls->parameter[pointCounter * 2 + OUTLINE_FIRST_X],
					   ls->parameter[pointCounter * 2 + OUTLINE_FIRST_Y]);
			}
			fprintf(fd, "%f*\n",ls->parameter[pointCounter * 2 + OUTLINE_FIRST_X]);
		}
		else if (ls->type == MACRO_POLYGON) {
			fprintf(fd, "5,%d,%d,%f,%f,%f,%f*\n",(int) ls->parameter[POLYGON_EXPOSURE],
				(int) ls->parameter[POLYGON_NUMBER_OF_POINTS],
				ls->parameter[POLYGON_CENTER_X],ls->parameter[POLYGON_CENTER_Y],
				ls->parameter[POLYGON_DIAMETER],ls->parameter[POLYGON_ROTATION]);
		}
		else if (ls->type == MACRO_MOIRE) {
			fprintf(fd, "6,%f,%f,%f,%f,%f,%d,%f,%f,%f*\n",ls->parameter[MOIRE_CENTER_X],
				ls->parameter[MOIRE_CENTER_Y],ls->parameter[MOIRE_OUTSIDE_DIAMETER],
				ls->parameter[MOIRE_CIRCLE_THICKNESS],ls->parameter[MOIRE_GAP_WIDTH],
				(int) ls->parameter[MOIRE_NUMBER_OF_CIRCLES],ls->parameter[MOIRE_CROSSHAIR_THICKNESS],
				ls->parameter[MOIRE_CROSSHAIR_LENGTH],ls->parameter[MOIRE_ROTATION]);
		}
		else if (ls->type == MACRO_THERMAL) {
			fprintf(fd, "7,%f,%f,%f,%f,%f,%f*\n",ls->parameter[THERMAL_CENTER_X],
				ls->parameter[THERMAL_CENTER_Y],ls->parameter[THERMAL_OUTSIDE_DIAMETER],
				ls->parameter[THERMAL_INSIDE_DIAMETER],ls->parameter[THERMAL_CROSSHAIR_THICKNESS],
				ls->parameter[THERMAL_ROTATION]);
		}
		else if (ls->type == MACRO_LINE20) {
			fprintf(fd, "20,%d,%f,%f,%f,%f,%f,%f*\n",(int) ls->parameter[LINE20_EXPOSURE],
				ls->parameter[LINE20_LINE_WIDTH],ls->parameter[LINE20_START_X],
				ls->parameter[LINE20_START_Y],ls->parameter[LINE20_END_X],
				ls->parameter[LINE20_END_Y],ls->parameter[LINE20_ROTATION]);
		}
		else if (ls->type == MACRO_LINE21) {
			fprintf(fd, "21,%d,%f,%f,%f,%f,%f*\n",(int) ls->parameter[LINE21_EXPOSURE],
				ls->parameter[LINE21_WIDTH],ls->parameter[LINE21_HEIGHT],
				ls->parameter[LINE21_CENTER_X],ls->parameter[LINE21_CENTER_Y],
				ls->parameter[LINE21_ROTATION]);
		}
		else if (ls->type == MACRO_LINE22) {
			fprintf(fd, "22,%d,%f,%f,%f,%f,%f*\n",(int) ls->parameter[LINE22_EXPOSURE],
				ls->parameter[LINE22_WIDTH],ls->parameter[LINE22_HEIGHT],
				ls->parameter[LINE22_LOWER_LEFT_X],ls->parameter[LINE22_LOWER_LEFT_Y],
				ls->parameter[LINE22_ROTATION]);
		}
		ls = ls->next;
	}
	fprintf(fd, "%%\n");
	/* and finally create an aperture definition to use the macro */
	fprintf(fd, "%%ADD%dMACRO%d*%%\n",apertureNumber,apertureNumber);
}

void
export_rs274x_write_apertures (FILE *fd, gerb_image_t *image) {
	gerb_aperture_t *currentAperture;
	gint numberOfRequiredParameters=0,numberOfOptionalParameters=0,i,j;
		
	for (i=APERTURE_MIN; i<APERTURE_MAX; i++) {
		gboolean writeAperture=TRUE;
		
		currentAperture = image->aperture[i];
		
		if (!currentAperture)
			continue;
		
		switch (currentAperture->type) {
			case CIRCLE:
				fprintf(fd, "%%ADD%d",i);
				fprintf(fd, "C,");
				numberOfRequiredParameters = 1;
				numberOfOptionalParameters = 2;
				break;
			case RECTANGLE:
				fprintf(fd, "%%ADD%d",i);
				fprintf(fd, "R,");
				numberOfRequiredParameters = 2;
				numberOfOptionalParameters = 2;
				break;
			case OVAL:
				fprintf(fd, "%%ADD%d",i);
				fprintf(fd, "O,");
				numberOfRequiredParameters = 2;
				numberOfOptionalParameters = 2;
				break;
			case POLYGON:
				fprintf(fd, "%%ADD%d",i);
				fprintf(fd, "P,");
				numberOfRequiredParameters = 2;
				numberOfOptionalParameters = 3;
				break;
			case MACRO:
				export_rs274x_write_macro (fd, currentAperture, i);
				writeAperture=FALSE;
				break;
			default:
				writeAperture=FALSE;
				break;
		}
		if (writeAperture) {
			/* write the parameter list */
			for (j=0; j<(numberOfRequiredParameters + numberOfOptionalParameters); j++) {
				if ((j < numberOfRequiredParameters) || (currentAperture->parameter[j] != 0)) {
					/* print the "X" character to separate the parameters */
					if (j>0)
						fprintf(fd, "X");
					fprintf(fd, "%.4f",currentAperture->parameter[j]);
				}
			}
			fprintf(fd, "*%%\n");
		}
	}
}

void
export_rs274x_write_layer_change (gerb_layer_t *oldLayer, gerb_layer_t *newLayer, FILE *fd) {
	if (oldLayer->polarity != newLayer->polarity) {
		/* polarity changed */
		if ((newLayer->polarity == CLEAR))
			fprintf(fd, "%%LPC*%%\n");
		else
			fprintf(fd, "%%LPD*%%\n");
	}
}

void
export_rs274x_write_state_change (gerb_netstate_t *oldState, gerb_netstate_t *newState, FILE *fd) {


}

gboolean
export_rs274x_file_from_image (gchar *filename, gerb_image_t *image) {
	FILE *fd;
	gerb_netstate_t *oldState;
	gerb_layer_t *oldLayer;
	
	if ((fd = g_fopen(filename, "w")) == NULL) {
		GERB_MESSAGE("Can't open file for writing: %s\n", filename);
		return FALSE;
	}
	/* write header info */
	fprintf(fd, "G04 Exported by Gerber Viewer version %s*\n",VERSION);
	fprintf(fd, "G04 --Header info--*\n");
	fprintf(fd, "%%MOIN*%%\n");
	fprintf(fd, "%%FSLAX23Y23*%%\n");
	
	/* check the image info struct for any non-default settings */
	/* image offset */
	if ((image->info->offsetA > 0.0) || (image->info->offsetB > 0.0))
		fprintf(fd, "%%IOA%fB%f*%%\n",image->info->offsetA,image->info->offsetB);
	/* image polarity */
	if (image->info->polarity == CLEAR)
		fprintf(fd, "%%IPNEG*%%\n");
	else
		fprintf(fd, "%%IPPOS*%%\n");
	/* image name */
	if (image->info->name)
		fprintf(fd, "%%IN%s*%%\n",image->info->name);
	/* plotter film */
	if (image->info->plotterFilm)
		fprintf(fd, "%%PF%s*%%\n",image->info->plotterFilm);
	/* image rotation */
	if (image->info->imageRotation != 0.0)
		fprintf(fd, "%%IR%d*%%\n",(int) image->info->imageRotation);
	if ((image->info->imageJustifyTypeA != NOJUSTIFY) ||
		(image->info->imageJustifyTypeB != NOJUSTIFY)) {
		fprintf(fd, "%%IJA");
		if (image->info->imageJustifyTypeA == CENTERJUSTIFY)
			fprintf(fd, "C");
		else 
			fprintf(fd, "%.4f",image->info->imageJustifyOffsetA);
		fprintf(fd, "B");
		if (image->info->imageJustifyTypeB == CENTERJUSTIFY)
			fprintf(fd, "C");
		else 
			fprintf(fd, "%.4f",image->info->imageJustifyOffsetB);
		fprintf(fd, "*%%\n");

	}
	/* define all apertures */
	fprintf(fd, "G04 --Define apertures--*\n");
	export_rs274x_write_apertures (fd, image);
	
	/* write rest of image */
	fprintf(fd, "G04 --Start main section--*\n");
	gint currentAperture = 0;
	gerb_net_t *currentNet;
	
	oldLayer = image->layers;
	oldState = image->states;
	for (currentNet = image->netlist; currentNet; currentNet = currentNet->next){
		/* check for "layer" changes (RS274X commands) */
		if (currentNet->layer != oldLayer)
			export_rs274x_write_layer_change (oldLayer, currentNet->layer, fd);
		
		/* check for new "netstate" (more RS274X commands) */
		if (currentNet->state != oldState)
			export_rs274x_write_state_change (oldState, currentNet->state, fd);
		
		/* check for tool changes */
		/* also, make sure the aperture number is a valid one, since sometimes
		   the loaded file may refer to invalid apertures */
		if ((currentNet->aperture != currentAperture)&&
			(image->aperture[currentNet->aperture] != NULL)) {
			fprintf(fd, "G54D%02d*\n",currentNet->aperture);
			currentAperture = currentNet->aperture;
		}
		
		oldLayer = currentNet->layer;
		oldState = currentNet->state;
		
		long xVal,yVal,endX,endY,centerX,centerY;;
		switch (currentNet->interpolation) {
			case LINEARx10 :
			case LINEARx01 :
			case LINEARx001 :
			case LINEARx1 :
				xVal = (long) round(currentNet->stop_x * 1000.0);
				yVal = (long) round(currentNet->stop_y * 1000.0);
				fprintf(fd, "G01X%05ldY%05ld",xVal,yVal);
				/* and finally, write the esposure value */
				if (currentNet->aperture_state == OFF)
					fprintf(fd, "D02*\n");
				else if (currentNet->aperture_state == ON)
					fprintf(fd, "D01*\n");
				else
					fprintf(fd, "D03*\n");
				break;
			case CW_CIRCULAR :
			case CCW_CIRCULAR :
				centerX= (long) round((currentNet->cirseg->cp_x - currentNet->start_x) * 1000.0);
				centerY= (long) round((currentNet->cirseg->cp_y - currentNet->start_y) * 1000.0);
				endX = (long) round(currentNet->stop_x * 1000.0);
				endY = (long) round(currentNet->stop_y * 1000.0);
				
				/* always use multi-quadrant, since it's much easier to export */
				/*  and most all software should support it */
				fprintf(fd, "G75*\n");
				/* figure out clockwise or c-clockwise */
				if (currentNet->cirseg->angle2 > currentNet->cirseg->angle1)
					fprintf(fd, "G03");
				else
					fprintf(fd, "G02");
				/* don't write the I and J values if the exposure is off */
				if (currentNet->aperture_state == ON)
					fprintf(fd, "X%05ldY%05ldI%05ldJ%05ld",endX,endY,centerX,centerY);
				else
					fprintf(fd, "X%05ldY%05ld",endX,endY);
				/* and finally, write the esposure value */
				if (currentNet->aperture_state == OFF)
					fprintf(fd, "D02*\n");
				else if (currentNet->aperture_state == ON)
					fprintf(fd, "D01*\n");
				else
					fprintf(fd, "D03*\n");
				break;
			case PAREA_START:
				fprintf(fd, "G36*\n");
				break;
			case PAREA_END:
				fprintf(fd, "G37*\n");
				break;
			default:
				break;
		}
	}
	
	/* write footer */
	fprintf(fd, "G04 --Footer info--*\n");
	fprintf(fd, "M02*\n");

	fclose(fd);
	return TRUE;
}