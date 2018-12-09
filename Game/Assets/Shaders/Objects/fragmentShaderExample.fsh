#version 330 core
		in vec4 ourColor;
		in vec2 ourTexCoord;
		out vec4 FragColor;
		uniform sampler2D ourTexture_0;
		iii
		void main()
		{
		     FragColor = texture(ourTexture_0, ourTexCoord);
		}