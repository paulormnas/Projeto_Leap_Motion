#ifndef SHADOWMAPPING_H_
#define SHADOWMAPPING_H_

class ShadowMapping {
	GLuint shadowMapTexture;
	int shadowMapWidth;
	int shadowMapHeight;

	GLfloat textureTrasnformS[4];
	GLfloat textureTrasnformT[4];
	GLfloat textureTrasnformR[4];
	GLfloat textureTrasnformQ[4];
private:
	void createDepthTexture(GLenum TesteFunc) {
		//Create the shadow map texture
    		glActiveTexture(GL_TEXTURE1);
		glGenTextures(1, &shadowMapTexture);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

		//Enable shadow comparison
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

		//Shadow comparison should be true (ie not in shadow) if r<=texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, TesteFunc);

		//Shadow comparison should generate an INTENSITY result
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

	}

	void loadTextureTransform() {
		GLfloat lightProjectionMatrix[16];
		GLfloat lightViewMatrix[16];

		glGetFloatv(GL_PROJECTION_MATRIX, lightProjectionMatrix);
		glGetFloatv(GL_MODELVIEW_MATRIX, lightViewMatrix);

		glPushAttrib(GL_TRANSFORM_BIT);
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();

		//Calculate texture matrix for projection
		//This matrix takes us from eye space to the light's clip space
		//It is postmultiplied by the inverse of the current view matrix when specifying texgen
		GLfloat biasMatrix[16]= {0.5f, 0.0f, 0.0f, 0.0f,
					    0.0f, 0.5f, 0.0f, 0.0f,
				 	    0.0f, 0.0f, 0.5f, 0.0f,
					    0.5f, 0.5f, 0.5f, 1.0f};	//bias from [-1, 1] to [0, 1]

		GLfloat textureMatrix[16];

		glLoadMatrixf(biasMatrix);
		glMultMatrixf(lightProjectionMatrix);
		glMultMatrixf(lightViewMatrix);
		glGetFloatv(GL_TEXTURE_MATRIX, textureMatrix);

		for (int i=0; i<4; i++) {
			textureTrasnformS[i] = textureMatrix[i*4];
			textureTrasnformT[i] = textureMatrix[i*4+1];
			textureTrasnformR[i] = textureMatrix[i*4+2];
			textureTrasnformQ[i] = textureMatrix[i*4+3];
		}

		glPopMatrix();
		glPopAttrib();
	}

public:
	void enableDepthCapture(GLenum TesteFunc) {
		glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);

		if (!shadowMapTexture)
			createDepthTexture(TesteFunc);

		glViewport(0, 0, shadowMapWidth, shadowMapHeight);
		loadTextureTransform();

		glPolygonOffset(20.0f, 10.0f);
		glEnable(GL_POLYGON_OFFSET_FILL);

		glShadeModel(GL_FLAT);
		glDisable(GL_LIGHTING);

		glColorMask(0, 0, 0, 0);
	}

	void disableDepthCapture() {
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, shadowMapWidth, shadowMapHeight);

		glCullFace(GL_BACK);
    		glShadeModel(GL_SMOOTH);
    		glColorMask(1, 1, 1, 1);

		glClear(GL_DEPTH_BUFFER_BIT);

		glPopAttrib();
	}

	void enableShadowTest() {
		glActiveTexture(GL_TEXTURE1);
		glPushAttrib(GL_TEXTURE_BIT |  GL_ENABLE_BIT);

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

		glTexGenfv(GL_S, GL_EYE_PLANE, textureTrasnformS);
		glTexGenfv(GL_T, GL_EYE_PLANE, textureTrasnformT);
		glTexGenfv(GL_R, GL_EYE_PLANE, textureTrasnformR);
		glTexGenfv(GL_Q, GL_EYE_PLANE, textureTrasnformQ);

		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_GEN_Q);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	}

	void disableShadowTest() {
		glPopAttrib();
	}

	ShadowMapping(int width = 1920, int height = 1080) {
		shadowMapTexture = 0;
		shadowMapWidth = width;
		shadowMapHeight = height;
	}
	virtual ~ShadowMapping() {
		shadowMapTexture = 0;
		glDeleteTextures(1, &shadowMapTexture);
	}
};

#endif /* SHADOWMAPPING_H_ */
