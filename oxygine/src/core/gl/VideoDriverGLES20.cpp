#include "VideoDriverGLES20.h"
#include "NativeTextureGLES.h"
#include "oxgl.h"
#include "math/Color.h"
#include "utils/stringUtils.h"
#include "../vertex.h"
#include "../files_io.h"
#include "../logging.h"



#ifdef __ANDROID__
#include "GLES2/gl2.h"
#endif

namespace oxygine
{	
	void printShaderInfoLog(GLuint shader)
	{
		GLint length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		if (length)
		{
			char* buffer = (char*)malloc( sizeof(char) * length ) ;
			glGetShaderInfoLog(shader, length, NULL, buffer);
			log::error("%s", buffer);
			free(buffer);

			GLint success;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (success != GL_TRUE) 
			{
				OX_ASSERT(!"shader build error");
				exit(1);
			}
		}
	}

	

	UberShaderProgram::UberShaderProgram(const char *fileName, const char *prepend, const char *append)		
	{
		file::read(fileName, _data);
		_data.data.insert(_data.data.begin(), prepend, prepend + strlen(prepend));
		_data.data.insert(_data.data.end(), append, append + strlen(append));
		_data.data.push_back(0);
	}



	UberShaderProgram::shader *UberShaderProgram::getShaderProgram(int flags)
	{
		shader &s = _shaders[flags];
				
		if (!s.program)
		{
			bvertex_format bformat = VERTEX_PCT2;
			s.flags = flags;
			const char* data = (const char*)_data.getData();
			
			char prepend[255];
			char append[255];

			append[0] = 0;
			prepend[0] = 0;

			if (flags & ALPHA_PREMULTIPLY)
				strcat(prepend, "#define ALPHA_PREMULTIPLY\n");

			if (flags & SEPARATE_ALPHA)
				strcat(prepend, "#define SEPARATE_ALPHA\n");

			if (flags & MASK)
			{
				strcat(prepend, "#define MASK\n");
				bformat = VERTEX_PCT2T2;
			}

			char *end = prepend + strlen(prepend);
			strcat(prepend, "#define program_main_ps main\n");
			strcat(prepend, "#define PS\n");

			int fs = createShader(GL_FRAGMENT_SHADER, data, prepend, append);

			*end = 0;
			strcat(prepend, "#define program_main_vs main\n");
			strcat(prepend, "#define VS\n");

			int vs = createShader(GL_VERTEX_SHADER, data, prepend, append);
					
			const VertexDeclarationGL *decl = ((VideoDriverGLES20*)IVideoDriver::instance)->getVertexDeclaration(bformat);
			ShaderProgramGL *pgl = new ShaderProgramGL;
			pgl->init(createProgram(vs, fs, decl));
			s.program = pgl;
		}

		return &s;
	}

	unsigned int UberShaderProgram::createShader(unsigned int type, const char* data, const char *prepend, const char *append)
	{
		GLuint shader = glCreateShader(type);

		const char *sources[16];
		const char **ptr = &sources[0];

		char nonGLES[] = 
			"#define lowp\n"
			"#define mediump\n";

#ifdef WIN32
		*ptr = nonGLES;
		ptr++;
#endif

		if (prepend)
		{
			*ptr = prepend;
			ptr++;
		}

		*ptr = data;
		ptr++;

		if (append)
		{
			*ptr = append;
			ptr++;
		}

		int num = ptr - sources;
		glShaderSource(shader, num, sources, 0);
		glCompileShader(shader);
		printShaderInfoLog(shader);

		return shader;
	}


	unsigned int UberShaderProgram::createProgram(int vs, int fs, const VertexDeclarationGL *decl)
	{
		int p = glCreateProgram();
		glAttachShader(p, vs);
		glAttachShader(p, fs);

		for (int i = 0; i < decl->numElements; ++i)		
			glBindAttribLocation(p, decl->elements[i].index, decl->elements[i].name);

		glLinkProgram(p);

		return p;
	}

	/*
	unsigned int VideoDriverGLES20::createShader(unsigned int type, const char* data, const char *prepend, const char *append)
	{
		GLuint shader = glCreateShader(type);

		const char *sources[3];
		const char **ptr = &sources[0];
		if (prepend)
		{
			*ptr = prepend;
			ptr++;
		}

		*ptr = data;
		ptr++;

		if (append)
		{
			*ptr = append;
			ptr++;
		}

		int num = ptr - sources;
		glShaderSource(shader, num, sources, 0);
		glCompileShader(shader);
		printShaderInfoLog(shader);

		return shader;
	}


	unsigned int VideoDriverGLES20::createProgram(int vs, int fs)
	{
		int p = glCreateProgram();
		glAttachShader(p, vs);
		glAttachShader(p, fs);
		
		const VertexDeclarationGL *decl = VertexDeclarationGL::getDeclaration(VERTEX_PCT2);
		for (int i = 0; i < decl->numElements; ++i)		
			glBindAttribLocation(p, decl->elements[i].index, decl->elements[i].name);

		glLinkProgram(p);

		return p;
	}
	*/

	VideoDriverGLES20::VideoDriverGLES20():_blend(blend_default), 
		_currentProgram(0), _us("system/shader.glsl")
	{
		file::buffer bf;
		file::read("system/shader.glsl", bf);
		bf.data.push_back(0);
		const char* data = (const char*)bf.getData();

		//ShaderProgram *s = us.getShaderProgram(UberShader::ALPHA_PREMULTIPLY);

		/*
		_vshader = createShader(GL_VERTEX_SHADER, data,
			"#define program_main_vs main\n"
			"#define VS\n", 0
			);

		_fshaderSplit = createShader(GL_FRAGMENT_SHADER, data, 
			"#define program_main_ps main\n"
			"#define SEPARATE_ALPHA\n"
			"#define ALPHA_PREMULTIPLY\n"
			"#define PS\n", 0);

		_fshaderMain = createShader(GL_FRAGMENT_SHADER, data, 
			"#define program_main_ps main\n"
			"#define PS\n", 0);

		_fshaderPremultiply = createShader(GL_FRAGMENT_SHADER, data, 
			"#define program_main_ps main\n"
			"#define ALPHA_PREMULTIPLY\n"
			"#define PS\n", 0);

			*/
		/*

		_splitShaderProgram = us.getShaderProgram(UberShader::SEPARATE_ALPHA | UberShader::ALPHA_PREMULTIPLY);
		_mainShaderProgram = us.getShaderProgram(0);		
		_premultiplyShaderProgram = us.getShaderProgram(UberShader::ALPHA_PREMULTIPLY);

		_currentShaderProgram = _mainShaderProgram;
		_currentShaderProgram->program->bind();
		*/
	}

	VideoDriverGLES20::~VideoDriverGLES20()
	{
		glUseProgram(0);
		/*

		glDeleteProgram(_mainShaderProgram);
		glDeleteProgram(_splitShaderProgram);
		glDeleteProgram(_premultiplyShaderProgram);

		glDeleteShader(_vshader);
		glDeleteShader(_fshaderMain);
		glDeleteShader(_fshaderSplit);		
		glDeleteShader(_fshaderPremultiply);		
		*/
	}

	spNativeTexture VideoDriverGLES20::createTexture()
	{
		return new NativeTextureGLES;
	}

	void VideoDriverGLES20::setProgram(ShaderProgramGL *p)
	{
		if (_currentProgram == p)
			return;

		_currentProgram = p;
		_currentProgram->bind();

		updateConstants();
	}

	void VideoDriverGLES20::updateConstants()
	{
		_currentProgram->setUniform("mat", &_matrixVP);
	}


	void VideoDriverGLES20::begin(const Matrix &proj, const Matrix &view, const Rect &viewport, const Color *clearColor)
	{
		_begin(viewport, clearColor);

		_matrixVP = view * proj;

		updateConstants();
	}
	
	void VideoDriverGLES20::bindTexture(int SAMPLER, const char *id, void* handle)
	{
		glActiveTexture(SAMPLER);
		glBindTexture(GL_TEXTURE_2D, (unsigned int)handle);	

		int alpha_location = glGetUniformLocation(_currentProgram->getID(), id);
		OX_ASSERT(alpha_location != -1);
		if (alpha_location != -1)
			glUniform1i(alpha_location, SAMPLER - GL_TEXTURE0);
	}	
	void VideoDriverGLES20::drawBatch(const batch &b)
	{		
		const VertexDeclarationGL *decl = static_cast<const VertexDeclarationGL*>(b.vdecl);

		int count = b.vertices.size() / decl->size;
		int indices = (count * 3)/2;
		const unsigned char *verticesData = &b.vertices[0];


		if (_blend != b.blend)
		{
			_blend = b.blend;

			glBlendEquation(GL_FUNC_ADD);
			bool blend = true;
			switch(b.blend)
			{
			case blend_disabled:
				blend = false;
				break;

			case blend_premultiplied_alpha:
				glBlendFunc  (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case blend_alpha:
				glBlendFunc  (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case blend_add:
				glBlendFunc  (GL_ONE, GL_ONE);
				break;
			case blend_sub:
				glBlendFunc  (GL_ONE, GL_ONE);
				glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
				break;
			default:
				OX_ASSERT(!"unknown blend");
			}				

			if (blend)
				glEnable(GL_BLEND);
			else
				glDisable(GL_BLEND);
		}		
		UberShaderProgram *program = b.program;
		if (program)		
		{
			int q=0;
		}
		else
		{
			program = &_us;
		}


		
		int flags = 0;
		if (!b.basePremultiplied)
			flags |= UberShaderProgram::ALPHA_PREMULTIPLY;
		if (b.alpha)
			flags |= UberShaderProgram::SEPARATE_ALPHA;
		if (b.mask)
			flags |= UberShaderProgram::MASK;

		ShaderProgramGL *glProg = program->getShaderProgram(flags)->program;


		setProgram(glProg);		

		bindTexture(GL_TEXTURE0, "base_texture", b.base->getHandle());

		if (b.alpha)
		{
			bindTexture(GL_TEXTURE1, "alpha_texture", b.alpha->getHandle());
		}
		if (b.mask)
		{
			bindTexture(GL_TEXTURE2, "mask_texture", b.mask->getHandle());
			glUniform4f(
				glGetUniformLocation(_currentProgram->getID(), "clip_mask"), 
				b.clipMask.getLeft(), b.clipMask.getTop(), b.clipMask.getRight(), b.clipMask.getBottom());
		}

		UberShaderProgram::ShaderUniformsCallback cb = program->getShaderUniformsCallback();
		if (cb)
		{
			cb(glProg);
		}
		
		const VertexDeclarationGL::Element *el = decl->elements;
		for (int i = 0; i < decl->numElements; ++i)		
		{
			glEnableVertexAttribArray(el->index);
			glVertexAttribPointer(el->index, el->size, el->elemType, el->normalized, decl->size, verticesData + el->offset);
			el++;
		}

		if (indices <= (int)_indices8.size())
			glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_BYTE, &_indices8[0]);
		else
			glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_SHORT, &_indices16[0]);
	}

	void VideoDriverGLES20::setDefaultSettings()
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glCullFace(GL_FRONT_AND_BACK);
		setProgram(_us.getShaderProgram(0)->program);
	}
}