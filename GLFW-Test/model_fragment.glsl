#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;//各種貼圖都active好了也送到shader中了 如果你有藥用再來這邊宣告來使用就可以
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

uniform sampler2D TEST;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform int shiness;
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;

/*struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};*/

//Material material;

void main()
{
	
	vec3 lightColor = vec3(1.0,1.0,1.0);
	
	float ambientStrengh = 0.1;
	vec3 ambient = ambientStrengh * lightColor  * texture(texture_diffuse1, TexCoords).rgb;// phong打環境光的得到的顏色

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse =Kd * diff * lightColor * texture(texture_diffuse1, TexCoords).rgb;// phong打漫反射光的得到的顏色

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shiness/4);
	vec3 specular = Ks * spec * lightColor * (texture(texture_specular1, TexCoords).rgb ) ;

	vec3 result = (ambient + diffuse + specular);

	FragColor = vec4(result, 1.0);
}