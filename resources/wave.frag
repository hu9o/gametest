uniform sampler2D texture;
uniform float time;
uniform vec2 origin;

void main()
{
    float intensity = 1.;
    
    vec2 input = gl_TexCoord[0].xy - origin;
    vec2 dist = vec2(.25-abs(input.x-.25), .25-abs(input.x-.25));
    //float offsetX = cos(input.y*3.14*2*20 + time/20) * (pow(dist.x, 2.)+pow(dist.y, 2.)) * 0.01 * intensity;
    float offsetY = sin(input.x*3.14*2*20 + time/20) * (pow(dist.x, 2.)+pow(dist.y, 2.)) * 0.01 * intensity;
    //float offsetY = sin(input.x*3.14*2*20 + time/400) * (pow(dist.x, 2.)) * 0.01 * intensity;

	gl_FragColor = texture2D(texture, gl_TexCoord[0].xy + vec2(0, offsetY));
}
