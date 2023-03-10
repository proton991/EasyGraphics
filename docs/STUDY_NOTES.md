# PBR

##### Radiant Flux

Radiant energy per time： $\phi=\frac{dQ}{dt}$ (unit: [W] or [J/S])

##### Radiant Intensity

Radiant flux per solid angle: $I = \frac{d\phi}{d\omega}$

e.g. Point Light: $I = \frac{\phi_{pointlight}}{4\pi}$

##### Irradiance

Radiant flux per area: $E = \frac{d\phi}{dA}$ (unit: [$W/m^2$]) 

e.g. A point light and distance $r$, the angle of incidence is $\theta$, we can calculate the irradiance:

$E = \frac{d\phi}{dA} = \frac{I d\omega}{dA}$

$d\omega = \frac{dS}{r^2}$

$S$ is the area $\perp$ light direction, so $dS = cos(\theta) dA$, finally, we get

$E = \frac{I\cos(\theta)}{r^2} = \frac{cos(\theta)\phi_{pointlight}}{4\pi r^2}$

* irradiance decreases quadratically with distance.
* irradiance decreases as the angle $\theta$ increases

##### Radiance

**Radiant flux** per **solid angle** per **projected area**: $L = \frac{d^2\phi}{d\omega\ cos(\theta)dA}$

**IMPORTANT: Does not change with viewing distance**.

$L = \frac{dE}{d\omega cos(\theta)}$ 

so $dE = L\ cos(\theta)\ d\omega$

$E = \int_{\Omega_{h}}L\cos(\theta)\ d\omega$

##### Rendering Equation

$$
L_{o}(v) = L_{e}(v)\ +\ \int_{\Omega_{h}}f_{r}(v,l)\ L_{i}\ cos(\theta)d\omega
$$

$L_o$: outgoing radiance

$L_i$: incoming radiance

$f_r$: BRDF $f_r(v,l)=\frac{dL_o(v)}{dE(l)}$




##### Phong BRDF

$$
f_{phong}(l,v,n)=k_{ambient} + k_{diffuse}(n\cdot l) + k_{specular}(r \cdot v)^{\alpha}
$$

* $\alpha$:  the shininess or roughness exponent, which determines the width of the specular lobe



##### Cook-Torrance Micro-facet BRDF

$$
f_r(v,l,n) = \frac{\rho_d}{\pi}(diffuse\ part) +\ \frac{F(v,h)\ D(h)\ G(l,v)}{4\ (n \cdot l)\ (n \cdot v)}(specular\ part)
$$

*Here we use half vector because in this BRDF model the surface is consisted of many micro faces, each tiny face is considered to be perfectly flat, which means we only receive light in view direction if the normal vector $n$ equals half vector $h$.*

1. Fresnel Reflectance

   Use Schlick approximation:  $F_{schlick}(v,h)=F_0 + (1-F_0)(1-(v \cdot h))^5$

   $F_0: vec3(0.04)$ for dielectrics; Use base color(albedo) for metals:

   ​	$F_0 = mix(F_0, baseColor, metallic);$

2. Normal Distribution Function

   * $h = \frac{(l+v)}{2}$

   * $r_p$: roughness in range [0.0, 1.0]
   * $\alpha = r_p^2$
   * $D_{GGX}(h)=\frac{\alpha^2}{\pi((n \cdot h)^2(\alpha^2-1)+1)^2}$

3. Geometry Term

   It is used to describe the attenuation of the light due to the microfacets shadowing each other. This is a statistical approximation again and it models the probability that at a given point the microfacets are occluded by each other or that light bounces on multiple microfacets, loosing energy in the process, before reaching the observer's eye.

   $G(l,v)=G_1(l)G_1(v)$ 

   * $G1_{Schlick-GGX}(v) = \frac{n \cdot v}{(n \cdot v)(1-k)+k}$
   * $G1_{Schlick-GGX}(l) = \frac{n \cdot l}{(n \cdot l)(1-k)+k}$
   * $\alpha$: same from NDF

4. Diffuse Part

   $\rho_d = baseColor *\ (1-metallic)*\ (1-F)$

   No diffuse for pure metals, and only transmitted light contributes to diffuse.

