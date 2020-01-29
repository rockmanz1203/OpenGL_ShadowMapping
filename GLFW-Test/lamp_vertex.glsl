#version 330 core
//使用的GL版本
layout (location = 0) in vec3 aPos;//aPos中保存頂點位置


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);

}