# GameAppリファレンス
`GameApp`(ゲーム側)から使える関数・変数のまとめ。関連：[[クラス構成]] / [[モデル読み込みの流れ]]

## 1. GameAppが書く関数(Gameの仮想関数)
`GameApp`は`Game`を継承し、4つの関数をoverrideして中身を書く。呼ぶのは`Engine::Run`。

| 関数 | 呼ばれるとき | 使いどころ |
|---|---|---|
| `Init(Engine&)` | 最初に1回(ウィンドウ表示の前) | 画像・形を読む、物を置く |
| `Update(Engine&, float dt)` | 毎フレーム | 動かす・ルール(`dt`=前フレームからの秒) |
| `Draw(Engine&)` | 毎フレーム(3D描画の後) | その場で描く(`DrawLine`など) |
| `DrawUI(Engine&)` | 毎フレーム(最後) | ImGuiのウィンドウ |

### 1フレームの順序
```
dt計算 → 窓サイズ変更の反映(最小化中は休んで次のループへ) → 入力取り込み
 → GameObjectのUpdate → game.Update → カメラ決定(F1で切替)
 → 画面クリア → 3D(Model)自動描画 → game.Draw → 線 → スプライト(2D)
 → ImGui開始 → game.DrawUI → ImGui終了 → 表示
```
- 2D(スプライト)は3Dより手前、ImGuiは一番手前

## 2. Engine(`engine.`で呼ぶ)

### 資源を読む(番号を返す)
| 関数 | 内容 |
|---|---|
| `int LoadTexture(const wchar_t* file)` | 画像を読む。例：`L"Resource/UVChecker0.png"` |
| `int LoadMesh(const char* file, float scale = 1.0f)` | モデル(obj, fbx等)を読む。失敗は`-1`。`scale`=単位合わせ(cm→mなら`0.01f`) |
| `int GetTriangleMesh() const` | 組み込みの三角形の形の番号 |
| `int GetQuadMesh() const` | 組み込みの四角形の形の番号 |

- 番号は`Model`・`Sprite`に渡す。同じ番号を何個でも使い回せる
- 画像はモデルのファイルから自動では読まれない。`LoadTexture`で別に読む

### 3Dの物を作る/消す(Engineが持ち、毎フレーム自動で描く)
| 関数 | 内容 |
|---|---|
| `Model* CreateModel(int mesh, int texture)` | `LoadMesh`した形で作る |
| `Model* CreateTriangle(int texture)` | 三角形 |
| `Model* CreateSphere(int texture)` | 球(半径0.5) |
| `void Destroy(Model* model)` | 削除。**以後そのポインタは使えない** |

- 返る`Model*`は**借りるだけ**。`delete`しない

### 2Dの物を作る/消す
| 関数 | 内容 |
|---|---|
| `Sprite* CreateSprite(int texture, Vector2 position = {640,360}, Vector2 size = {100,100})` | スプライト(ピクセル座標) |
| `void Destroy(Sprite* sprite)` | 削除 |

### GameObjectを出す
| 関数 | 内容 |
|---|---|
| `T* Spawn<T>(args...)` | `GameObject`の派生クラスを出す。引数は`T`のコンストラクタへ渡る |

```cpp
engine.Spawn<Spinner>(texture0, 0.0f, 1.0f);   //Spinner(texture, x, y)を出す
```
- `Init()`はSpawnの中で呼ばれ、`Update()`が動き出すのは**次のフレームから**

### その場で描く(`Draw`の中で呼ぶ。普段は不要)
| 関数 | 内容 |
|---|---|
| `DrawMesh(int mesh, int texture, const Transform&, const Vector4& color)` | 3Dを1回描く |
| `DrawSprite(Vector2 position, Vector2 size, float rotation, int texture, Vector4 color)` | 2Dを1回描く(`position`=中心、`rotation`=度) |

### 確認用の線(そのフレームだけ表示。毎フレーム呼ぶ)
| 関数 | 内容 |
|---|---|
| `DrawLine(Vector3 start, Vector3 end, Vector4 color)` | ワールド座標・1ピクセル幅の線 |
| `DrawGrid(float size = 10, int divisions = 10, Vector4 color = 灰)` | y=0の床に、中心が原点で一辺`size`の格子 |
| `DrawBox(Vector3 center, Vector3 size, Vector4 color)` | 軸にそろった箱の枠(当たり判定の表示用) |

- 1フレームに描ける線の頂点は最大65536個(超えるとログに警告)

### カメラ・入力・ウィンドウ
| 関数 | 内容 |
|---|---|
| `Camera& GetCamera()` | ゲームのカメラ(動かす用) |
| `DebugCamera& GetDebugCamera()` | デバッグカメラ(**F1で切り替え**) |
| `const Input& GetInput() const` | キー・マウス入力(読むだけ) |
| `SetWindowSize(UINT w, UINT h)` | 描画範囲の大きさ(次のフレームで反映) |
| `UINT GetWindowWidth() const` / `GetWindowHeight() const` | 窓の大きさ |

## 3. Model(3Dの物)
`CreateXxx`が返す`Model*`の、メンバ。

| メンバ | 型 | 内容 |
|---|---|---|
| `transform` | `Transform` | 位置・回転・拡大(親も指定可) |
| `color` | `Vector4` | RGBA。画像に掛ける(白=画像のまま) |
| `texture` | `int` | 画像の番号 |
| `visible` | `bool` | falseなら描かない |

- 形の番号(`m_mesh`)は`private`。作ったあとは変えられない

## 4. Sprite(2Dの物)
`CreateSprite`が返す`Sprite*`の、メンバ。

| メンバ | 型 | 内容 |
|---|---|---|
| `position` | `Vector2` | 中心のピクセル座標(左上が0,0) |
| `size` | `Vector2` | ピクセルサイズ |
| `rotation` | `float` | 回転(度) |
| `color` | `Vector4` | RGBA |
| `texture` | `int` | 画像の番号 |
| `visible` | `bool` | falseなら描かない |

## 5. GameObject(動く物の基底クラス)
`Spawn<T>`で出す物の親。派生クラスで中身を書く。

| 関数・変数 | 内容 |
|---|---|
| `virtual void Init()` | 出現したとき1回。ここで`Model`を作れる |
| `virtual void Update(float dt)` | 毎フレーム |
| `void Destroy()` | 削除の予約(フレームの終わりに消える) |
| `bool IsAlive() const` | 削除予約されていないか |
| `Transform transform` | この物の位置(作った`Model`はこれに付いてくる) |
| `Engine& GetEngine()` *(protected)* | エンジンを使う |
| `CreateModel` / `CreateTriangle` / `CreateSphere` / `CreateSprite` *(protected)* | 作った物を記録する。**GameObjectが消えると自動で削除**、`Model`は`transform`が親になる |

- `Engine`の`CreateXxx`との違い：**後始末が自動**で、**親子関係が付く**

## 6. Input(`engine.GetInput()`)
| 関数 | 内容 |
|---|---|
| `bool IsDown(int key)` | 押されている間ずっと |
| `bool IsPressed(int key)` | 押された瞬間(1フレームだけ) |
| `bool IsReleased(int key)` | 離された瞬間(1フレームだけ) |
| `Vector2 GetMousePosition()` | 窓内のピクセル座標(左上0,0) |
| `Vector2 GetMouseDelta()` | 前フレームからの移動量 |
| `float GetWheel()` | このフレームのホイール量(1=1目盛り、奥が+) |

- `key`はWindowsの仮想キーコード：`'A'`、`VK_SPACE`、`VK_LEFT`、マウスは`VK_LBUTTON`など

```cpp
if (engine.GetInput().IsPressed(VK_SPACE)) { /*弾を出す*/ }
```

## 7. カメラ
### Camera(`engine.GetCamera()`)
| メンバ・関数 | 内容 |
|---|---|
| `Transform transform` | 位置・回転(度)。初期位置`(0,0,-3)`、+zが前。`parent`で、物に付いていける |
| `float fovY` | 縦の視野角(度)。初期値60 |
| `float nearZ` / `farZ` | 手前・奥の限界(0.1 / 100) |
| `GetViewMatrix()` / `GetProjectionMatrix(aspect)` / `GetViewProjectionMatrix(aspect)` | 行列(普段は使わない) |

### DebugCamera(`engine.GetDebugCamera()`)
- **F1で切り替え**。右ドラッグ=回転、ホイール=距離

| メンバ | 内容 |
|---|---|
| `bool enabled` | trueの間、ゲームのカメラの代わりに使う |
| `Vector3 target` | 回る中心 |
| `float yaw` / `pitch` / `distance` | 横の角度・縦の角度・中心からの距離 |

## 8. Transform
| メンバ | 型 | 内容 |
|---|---|---|
| `position` | `XMFLOAT3` | 移動 |
| `rotation` | `XMFLOAT3` | 回転(度) xyz |
| `scale` | `XMFLOAT3` | 拡大 |
| `parent` | `const Transform*` | 親(nullptrなら親なし)。親の動きに付いてくる |

- `GetWorldMatrix()`：S→R→T→親の順で、ローカル→ワールドの行列

## 9. 数学(Vector)
`#include "Math/Vector.h"`で、`Vector2`・`Vector3`・`Vector4`が使える。

### 共通の演算(Vector2 / 3 / 4)
| 書き方 | 内容 |
|---|---|
| `a + b` / `a - b` | 加算・減算 |
| `a * b` / `a / b` | **成分ごと**の掛け算・割り算 |
| `a * s` / `a / s` | スカラー倍・スカラー除算(`s`はfloat) |
| `-a` | 符号反転 |
| `a.Dot(b)` | 内積 |
| `a.Length()` | 長さ |
| `a.Normalize()` | 長さ1にしたベクトル(長さ0なら0のまま) |

### 型ごとの違い
| 型 | 追加 | 注意 |
|---|---|---|
| `Vector2` | `Cross`(**スカラー**を返す) | 位置・サイズ・UVなどに |
| `Vector3` | `Cross`(ベクトルを返す) | 位置・方向。`DrawLine`などで使う |
| `Vector4` | なし | 色(RGBA)に使う。`x,y,z,w`＝`r,g,b,a` |

- `Vector3 * float`は書けるが、**`float * Vector3`は書けない**(`2.0f * v`はエラー)
- `+=`などの複合代入は**ない**(`a = a + b`と書く)

### 数学(Matrix)
`#include "Math/Matrix.h"`で、`Matrix3x3`・`Matrix4x4`が使える。`m[行][列]`で中身に触れる。
- 行ベクトル方式(`v × M`)。移動量は`Matrix4x4`の**最下行**(`m[3][0..2]`)に入る
- 今のエンジン(`Transform`・`Camera`)は**DirectXMathを使っている**。この2つは、自分で計算したいとき用(エンジンの描画には使われない)

| 書き方 | 対象 | 内容 |
|---|---|---|
| `Matrix4x4::Identity()` / `Matrix3x3::Identity()` | 両方 | 単位行列(何もしない行列) |
| `a * b` | 両方 | 行列の積。**左から順に適用**(`S * R * T`) |
| `a + b` / `a - b` / `a * s` | 両方 | 加算・減算・スカラー倍 |
| `m.Transpose()` | 両方 | 転置 |
| `m.Determinant()` | `Matrix3x3` | 行列式 |
| `m.Transform(v)` | 両方 | ベクトルに適用(`Matrix4x4`は`Vector4`、`Matrix3x3`は`Vector3`) |
| `m.TransformPoint(p)` | `Matrix4x4` | 位置に適用(w=1。**移動が効く**) |
| `m.TransformDirection(d)` | `Matrix4x4` | 方向に適用(w=0。**移動は効かない**) |
| `Matrix4x4::Translation(Vector3)` | `Matrix4x4` | 移動行列 |
| `Matrix4x4::Scaling(Vector3)` / `Matrix3x3::Scaling(Vector3)` | 両方 | 拡大行列 |
| `RotationX(rad)` / `RotationY(rad)` / `RotationZ(rad)` | 両方 | 軸まわりの回転行列。**角度はラジアン**(`Transform`は度) |

```cpp
Matrix4x4 world = Matrix4x4::Scaling({ 2, 2, 2 }) * Matrix4x4::RotationY(3.14159f / 2) * Matrix4x4::Translation({ 1, 0, 0 });
Vector3 p = world.TransformPoint({ 0, 0, 1 });   //ローカルの点をワールドへ
```

## 10. ログとImGui
### Log(`#include "Core/Log.h"`。`Common.h`経由で読まれる)
| 関数 | 内容 |
|---|---|
| `Log::Info(fmt, ...)` | 普通の報告 |
| `Log::Warn(fmt, ...)` | 動くが怪しい |
| `Log::Error(fmt, ...)` | 失敗 |

- `printf`と同じ書き方。出力ウィンドウと`log.txt`に出る

```cpp
Log::Info("beetle mesh = %d", beetleMesh);
```

### ImGui(`DrawUI`の中で使う)
- `#include "imgui.h"`。`ImGui::Begin("名前")`～`ImGui::End()`で窓を作る
- 例：`ImGui::DragFloat3("Position", &model->transform.position.x, 0.01f)`、`ImGui::ColorEdit4("Color", &model->color.x)`
- `Vector`・`XMFLOAT3`は、`x`のアドレスを渡せば、`DragFloat3`などで編集できる(メンバが連続しているため)

## 11. よくある書き方
```cpp
//Init：読む→置く
int tex  = engine.LoadTexture(L"Resource/UVChecker0.png");
int mesh = engine.LoadMesh("Resource/…/Beatall.fbx", 0.01f);
Model* car = engine.CreateModel(mesh, tex);
car->transform.position = { 0.6f, -0.6f, 0.0f };
car->color = { 1.0f, 0.5f, 0.5f, 1.0f };

//Update：動かす
car->transform.rotation.y += 90.0f * dt;          //1秒に90度回す
if (engine.GetInput().IsDown(VK_RIGHT)) car->transform.position.x += 1.0f * dt;

//Draw：確認用の線
engine.DrawGrid();
engine.DrawBox({ 0, 0, 0 }, { 1, 1, 1 }, { 1, 0, 0, 1 });
```
- **`dt`を掛ける**と、フレームレートに関係なく、同じ速さで動く(`速さ × dt`)

## 12. 注意点
- `Model*`・`Sprite*`は**借りているだけ**。`delete`しない。`Destroy`したら、そのポインタは使わない
- `LoadMesh`が`-1`を返したら失敗。そのまま`CreateModel`しても、描かれないだけで落ちない(理由は`log.txt`)
- `texture`に`-1`の`Model`は、描くと範囲外アクセスになる(画像は必ず読んでから渡す)
- 3Dの位置は**ワールド座標**(カメラ初期位置`(0,0,-3)`から原点を見る)、2Dは**ピクセル座標**(左上が`(0,0)`、y下向き)
- 回転は**度**で持つ(`Transform::rotation`、`Sprite::rotation`)
