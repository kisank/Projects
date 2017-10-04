in vec4 position;

void main()
{
gl_FragData[0]=position.w/100;
}