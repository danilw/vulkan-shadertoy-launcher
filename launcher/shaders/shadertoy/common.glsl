// no arrays and index const to avoid all bugs
// fixed pow and float printing, it correct no loss because pow bugs

int get_fvalue(float val, int idx){
    for(int i=0;i<idx+1;i++){
        val=(val-floor(val/10.)*10.)*10.;
    }
    return int(val/10.);
}

float getarr(in mat3 arr, int idx){
idx=max(idx,0);ivec2 ij=ivec2((idx/3)%3,idx%3);
if(ij==ivec2(0,0))return arr[0][0];
if(ij==ivec2(0,1))return arr[0][1];
if(ij==ivec2(0,2))return arr[0][2];
if(ij==ivec2(1,0))return arr[1][0];
if(ij==ivec2(1,1))return arr[1][1];
if(ij==ivec2(1,2))return arr[1][2];
if(ij==ivec2(2,0))return arr[2][0];
if(ij==ivec2(2,1))return arr[2][1];
if(ij==ivec2(2,2))return arr[2][2];
return arr[0][0];}

float print_num(vec2 uv, float value, int num) {
    const mat3 fontb=mat3(vec3(480599.0,139810.0,476951.0),vec3(476999.0,350020.0,464711.0),vec3(464727.0,476228.0,481111.0));
    const mat3 powers = mat3(vec3(1., 10., 100.), vec3(1000., 10000., 100000.), vec3(1000000., 10000000., 100000000.));
  if(uv.y < 0.0 || uv.y >= 1.0) return 0.0;
    if(uv.x < -6.0 || uv.x >= 10.0) return 0.0;
  float bits = 0.0;
  int di = - int(floor(uv.x))+ 1;
  if(-di <= num) {
    float pw = getarr(powers,di);
    float val = abs(value);
    float pivot = max(val, 1.5) * 10.0;
    if(pivot < pw) {
      if(value < 0.0 && pivot >= pw * 0.1) bits = 1792.0;
    } else {
            if(di == 0) {
                if(num > 0) bits = 2.0;
            } else {
                int idx=0;
                if(di < 0)idx=get_fvalue(val,int(-di));else idx=(int(val*10.) / int(pw))%10;
                if(idx<=9 && idx>=0)bits = idx<9?getarr(fontb,idx):481095.0;
            }
        }
  } else return 0.;
  return floor(mod(bits / exp2(floor(fract(uv.x) * 4.0) + floor(uv.y * 5.0) * 4.0), 2.0));
}

vec3 print_n(in vec2 uv ,float val){
    int numbers = 6;
        vec2 font = vec2(50.)/vec2(1280,720);
    float d = print_num(uv/font, val, numbers);
        return vec3(1.0, 1.0, 1.0)* d;
}
