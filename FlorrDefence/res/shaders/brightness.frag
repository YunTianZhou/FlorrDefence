uniform sampler2D texture;
uniform vec4 flashColor;   // Typically from sf::Sprite::setColor(...)
uniform float brightness;  // 0.0 = normal, 1.0 = fully white

void main()
{
    vec4 baseColor = texture2D(texture, gl_TexCoord[0].xy);

    // Modulate color as SFML does
    vec4 modulated = baseColor * flashColor;

    // Brighten RGB only, leave alpha untouched
    vec3 brightRGB = mix(modulated.rgb, vec3(1.0), brightness);

    gl_FragColor = vec4(brightRGB, modulated.a);
}
