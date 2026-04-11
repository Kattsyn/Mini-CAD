#include "viewer3d.h"
#include "commands3d.h"

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>

#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QTorusMesh>

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QFrustumCulling>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QNoDraw>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QViewport>

#include <QTimer>
#include <QUndoStack>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

Viewer3D::Viewer3D(QWidget *parent)
    : QWidget(parent)
    , m_undoStack(new QUndoStack(this))
{
    m_window3d = new Qt3DExtras::Qt3DWindow();

    QWidget *container = QWidget::createWindowContainer(m_window3d, this);
    container->setMinimumSize(400, 300);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setFocusPolicy(Qt::StrongFocus);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(container);

    setupScene();
}

// ─────────────────────────────────────────────────────────────────────────────
// Scene setup
// ─────────────────────────────────────────────────────────────────────────────

void Viewer3D::setupScene()
{
    m_rootEntity  = new Qt3DCore::QEntity();
    m_gizmoLayer  = new Qt3DRender::QLayer(m_rootEntity);

    // Create gizmo camera before the framegraph (framegraph references it)
    m_gizmoCamera = new Qt3DRender::QCamera(m_rootEntity);
    m_gizmoCamera->lens()->setPerspectiveProjection(50.0f, 1.0f, 0.1f, 100.0f);
    m_gizmoCamera->setPosition(QVector3D(0.0f, 0.0f, 5.0f));
    m_gizmoCamera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    m_gizmoCamera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));

    setupFrameGraph();

    // ── Main camera ───────────────────────────────────────────────────────────
    Qt3DRender::QCamera *camera = m_window3d->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(8.0f, 6.0f, 12.0f));
    camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

    // Orbit controller
    auto *ctrl = new Qt3DExtras::QOrbitCameraController(m_rootEntity);
    ctrl->setCamera(camera);
    ctrl->setLinearSpeed(25.0f);
    ctrl->setLookSpeed(180.0f);

    // Keep gizmo camera in sync with main camera rotation
    connect(camera, &Qt3DRender::QCamera::viewMatrixChanged,
            this, [this, camera]() {
        syncGizmoCamera(camera->viewMatrix());
    });
    syncGizmoCamera(camera->viewMatrix());

    setupLights();
    setupGrid();
    setupGizmo();

    m_shapesRoot = new Qt3DCore::QEntity(m_rootEntity);
    m_window3d->setRootEntity(m_rootEntity);
}

// ── Custom two-viewport framegraph ────────────────────────────────────────────
//
//  surfaceSelector
//  ├── mainViewport (full screen)
//  │   ├── clearBuffers (color+depth) → NoDraw
//  │   └── mainCamSel → layerFilter(discard gizmo) → frustumCulling  [leaf]
//  └── gizmoViewport (bottom-left 18%)
//      ├── gizmoClear (depth only)  → NoDraw
//      └── gizmoCamSel → layerFilter(accept gizmo)                   [leaf]

void Viewer3D::setupFrameGraph()
{
    using namespace Qt3DRender;

    auto *surf = new QRenderSurfaceSelector(m_rootEntity);
    surf->setSurface(m_window3d);

    // ── Main viewport (full screen) ───────────────────────────────────────────
    auto *mainVP = new QViewport(surf);
    mainVP->setNormalizedRect(QRectF(0.0, 0.0, 1.0, 1.0));

    // Clear colour + depth
    auto *clear = new QClearBuffers(mainVP);
    clear->setBuffers(QClearBuffers::ColorDepthBuffer);
    clear->setClearColor(QColor(45, 45, 48));
    new QNoDraw(clear);

    // Camera → layer filter (exclude gizmo) → frustum culling (leaf = renders)
    auto *mainCam = new QCameraSelector(mainVP);
    mainCam->setCamera(m_window3d->camera());

    auto *mainFilter = new QLayerFilter(mainCam);
    mainFilter->addLayer(m_gizmoLayer);
    mainFilter->setFilterMode(QLayerFilter::DiscardAnyMatchingLayers);

    new QFrustumCulling(mainFilter);   // leaf → triggers main scene render

    // ── Gizmo viewport (bottom-left corner, 18 % × 18 %) ─────────────────────
    auto *gizmoVP = new QViewport(surf);
    gizmoVP->setNormalizedRect(QRectF(0.0, 0.82, 0.18, 0.18));

    // Clear only depth so gizmo paints over the main scene
    auto *gizmoClear = new QClearBuffers(gizmoVP);
    gizmoClear->setBuffers(QClearBuffers::DepthBuffer);
    new QNoDraw(gizmoClear);

    // Camera → layer filter (only gizmo entities) → leaf render
    m_gizmoCamSel = new QCameraSelector(gizmoVP);
    m_gizmoCamSel->setCamera(m_gizmoCamera);

    auto *gizmoFilter = new QLayerFilter(m_gizmoCamSel);
    gizmoFilter->addLayer(m_gizmoLayer);
    gizmoFilter->setFilterMode(QLayerFilter::AcceptAnyMatchingLayers);
    // gizmoFilter has no children → it IS the leaf → triggers gizmo render

    m_window3d->setActiveFrameGraph(surf);
}

// ── Sync gizmo camera rotation to main camera ─────────────────────────────────

void Viewer3D::syncGizmoCamera(const QMatrix4x4 &viewMatrix)
{
    if (!m_gizmoCamera) return;

    // Strip translation — keep only the rotation part
    QMatrix4x4 rotOnly = viewMatrix;
    rotOnly(0, 3) = 0.0f;
    rotOnly(1, 3) = 0.0f;
    rotOnly(2, 3) = 0.0f;

    // Gizmo camera sits at world position = inv(rotation) * (0,0,5)
    const QMatrix4x4 invRot = rotOnly.inverted();
    const QVector3D  pos    = invRot.map(QVector3D(0.0f, 0.0f, 5.0f));
    const QVector3D  up     = invRot.mapVector(QVector3D(0.0f, 1.0f, 0.0f));

    m_gizmoCamera->setPosition(pos);
    m_gizmoCamera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    m_gizmoCamera->setUpVector(up.normalized());
}

// ── Lights ────────────────────────────────────────────────────────────────────

void Viewer3D::setupLights()
{
    auto *lightEnt = new Qt3DCore::QEntity(m_rootEntity);
    auto *light    = new Qt3DRender::QDirectionalLight(lightEnt);
    light->setColor(Qt::white);
    light->setIntensity(1.2f);
    light->setWorldDirection(QVector3D(-1.0f, -2.0f, -1.0f));
    lightEnt->addComponent(light);

    auto *fillEnt = new Qt3DCore::QEntity(m_rootEntity);
    auto *fill    = new Qt3DRender::QDirectionalLight(fillEnt);
    fill->setColor(QColor(200, 200, 255));
    fill->setIntensity(0.4f);
    fill->setWorldDirection(QVector3D(1.0f, 1.0f, 0.5f));
    fillEnt->addComponent(fill);
}

// ── Ground grid ───────────────────────────────────────────────────────────────

void Viewer3D::setupGrid()
{
    auto *gridEnt = new Qt3DCore::QEntity(m_rootEntity);

    auto *mesh = new Qt3DExtras::QCuboidMesh(gridEnt);
    mesh->setXExtent(20.0f);
    mesh->setYExtent(0.02f);
    mesh->setZExtent(20.0f);
    gridEnt->addComponent(mesh);

    auto *mat = new Qt3DExtras::QPhongMaterial(gridEnt);
    mat->setDiffuse(QColor(80, 80, 80));
    mat->setAmbient(QColor(60, 60, 60));
    mat->setShininess(0);
    gridEnt->addComponent(mat);

    auto *t = new Qt3DCore::QTransform(gridEnt);
    t->setTranslation(QVector3D(0.0f, -0.01f, 0.0f));
    gridEnt->addComponent(t);
}

// ── Axis gizmo (rendered in the corner viewport) ──────────────────────────────

void Viewer3D::setupGizmo()
{
    // Helper: one axis arrow = thin cylinder body + cone tip
    // Default QCylinderMesh is along Y. Rotation aligns it to the desired axis.
    struct AxisDef { QColor color; QVector3D rotAxis; float rotDeg; };
    const AxisDef axes[3] = {
        { QColor(220,  60,  60), QVector3D(0, 0, 1), -90.0f },  // X  — rotate around Z
        { QColor( 60, 200,  60), QVector3D(1, 0, 0),   0.0f },  // Y  — no rotation
        { QColor( 60, 120, 220), QVector3D(1, 0, 0),  90.0f },  // Z  — rotate around X
    };
    // Axis tip positions (world-space end of each arrow, before rotation)
    const QVector3D tips[3] = {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
    };
    Q_UNUSED(tips)

    const float cylLen    = 1.4f;
    const float cylRad    = 0.04f;
    const float coneLen   = 0.35f;
    const float coneRad   = 0.10f;

    for (int i = 0; i < 3; ++i) {
        const QQuaternion rot = (axes[i].rotDeg != 0.0f)
            ? QQuaternion::fromAxisAndAngle(axes[i].rotAxis, axes[i].rotDeg)
            : QQuaternion{};

        // ── Cylinder body ─────────────────────────────────────────────────────
        auto *cylEnt = new Qt3DCore::QEntity(m_rootEntity);
        cylEnt->addComponent(m_gizmoLayer);

        auto *cylMesh = new Qt3DExtras::QCylinderMesh(cylEnt);
        cylMesh->setRadius(cylRad);
        cylMesh->setLength(cylLen);
        cylMesh->setRings(2);
        cylMesh->setSlices(16);
        cylEnt->addComponent(cylMesh);

        auto *cylMat = new Qt3DExtras::QPhongMaterial(cylEnt);
        cylMat->setDiffuse(axes[i].color);
        cylMat->setAmbient(axes[i].color.darker(150));
        cylMat->setShininess(20);
        cylEnt->addComponent(cylMat);

        auto *cylTf = new Qt3DCore::QTransform(cylEnt);
        // Cylinder centre is at half its length along the rotated axis
        QVector3D centre(0.0f, cylLen * 0.5f, 0.0f);
        centre = rot.rotatedVector(centre);
        cylTf->setRotation(rot);
        cylTf->setTranslation(centre);
        cylEnt->addComponent(cylTf);

        // ── Cone tip ──────────────────────────────────────────────────────────
        auto *coneEnt = new Qt3DCore::QEntity(m_rootEntity);
        coneEnt->addComponent(m_gizmoLayer);

        auto *coneMesh = new Qt3DExtras::QConeMesh(coneEnt);
        coneMesh->setBottomRadius(coneRad);
        coneMesh->setTopRadius(0.0f);
        coneMesh->setLength(coneLen);
        coneMesh->setRings(2);
        coneMesh->setSlices(16);
        coneEnt->addComponent(coneMesh);

        auto *coneMat = new Qt3DExtras::QPhongMaterial(coneEnt);
        coneMat->setDiffuse(axes[i].color);
        coneMat->setAmbient(axes[i].color.darker(150));
        coneEnt->addComponent(coneMat);

        auto *coneTf = new Qt3DCore::QTransform(coneEnt);
        // Cone base sits at the cylinder tip
        QVector3D coneCentre(0.0f, cylLen + coneLen * 0.5f, 0.0f);
        coneCentre = rot.rotatedVector(coneCentre);
        coneTf->setRotation(rot);
        coneTf->setTranslation(coneCentre);
        coneEnt->addComponent(coneTf);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Shape rebuild
// ─────────────────────────────────────────────────────────────────────────────

void Viewer3D::rebuildShapes()
{
    for (auto *child : m_shapesRoot->children()) {
        if (auto *ent = qobject_cast<Qt3DCore::QEntity*>(child)) {
            ent->setEnabled(false);  // immediately removes from render + picking
            ent->deleteLater();
        }
    }

    for (int i = 0; i < m_shapes.size(); ++i)
        buildEntity(m_shapes[i], i, m_shapesRoot);

    emit shapesChanged();
}

Qt3DCore::QEntity *Viewer3D::buildEntity(const Shape3DParams &p, int index,
                                          Qt3DCore::QEntity *parent)
{
    auto *ent = new Qt3DCore::QEntity(parent);

    // ── Mesh ──────────────────────────────────────────────────────────────────
    switch (p.type) {
        case ShapeType3D::Box: {
            auto *m = new Qt3DExtras::QCuboidMesh(ent);
            m->setXExtent(p.xSize);
            m->setYExtent(p.ySize);
            m->setZExtent(p.zSize);
            ent->addComponent(m);
            break;
        }
        case ShapeType3D::Sphere: {
            auto *m = new Qt3DExtras::QSphereMesh(ent);
            m->setRadius(p.xSize);
            m->setRings(32); m->setSlices(32);
            ent->addComponent(m);
            break;
        }
        case ShapeType3D::Cylinder: {
            auto *m = new Qt3DExtras::QCylinderMesh(ent);
            m->setRadius(p.xSize);
            m->setLength(p.ySize);
            m->setRings(2); m->setSlices(32);
            ent->addComponent(m);
            break;
        }
        case ShapeType3D::Cone: {
            auto *m = new Qt3DExtras::QConeMesh(ent);
            m->setBottomRadius(p.xSize);
            m->setTopRadius(p.radius2);
            m->setLength(p.ySize);
            m->setRings(2); m->setSlices(32);
            ent->addComponent(m);
            break;
        }
        case ShapeType3D::Torus: {
            auto *m = new Qt3DExtras::QTorusMesh(ent);
            m->setRadius(p.xSize);
            m->setMinorRadius(p.radius2);
            m->setRings(48); m->setSlices(16);
            ent->addComponent(m);
            break;
        }
    }

    // ── Material ──────────────────────────────────────────────────────────────
    const bool selected = (index == m_selectedIndex);
    auto *mat = new Qt3DExtras::QPhongMaterial(ent);
    mat->setDiffuse(selected ? p.color.lighter(160) : p.color);
    mat->setAmbient(selected ? p.color.lighter(120) : p.color.darker(200));
    mat->setSpecular(QColor(255, 255, 255));
    mat->setShininess(selected ? 100.0f : 30.0f);
    ent->addComponent(mat);

    // ── Transform ─────────────────────────────────────────────────────────────
    auto *tf = new Qt3DCore::QTransform(ent);
    tf->setTranslation(p.position);
    tf->setScale3D(p.scale);
    tf->setRotation(QQuaternion::fromEulerAngles(p.rotation.x(),
                                                  p.rotation.y(),
                                                  p.rotation.z()));
    ent->addComponent(tf);

    // ── Object picker (click-to-select) ───────────────────────────────────────
    auto *picker = new Qt3DRender::QObjectPicker(ent);
    picker->setHoverEnabled(false);
    ent->addComponent(picker);

    connect(picker, &Qt3DRender::QObjectPicker::pressed,
            this, [this, index](Qt3DRender::QPickEvent *) {
        setSelectedIndex(index);
    });

    return ent;
}

// ─────────────────────────────────────────────────────────────────────────────
// Selection
// ─────────────────────────────────────────────────────────────────────────────

void Viewer3D::setSelectedIndex(int index)
{
    if (m_selectedIndex == index) return;
    m_selectedIndex = index;
    emit selectionChanged(index);
    // Defer rebuild so Qt3D finishes processing the picker event first.
    // Calling rebuildShapes() synchronously from within QObjectPicker::pressed
    // can corrupt the picking state (entity disabled while still in event chain).
    QTimer::singleShot(0, this, &Viewer3D::rebuildShapes);
}

// ─────────────────────────────────────────────────────────────────────────────
// Shape queries
// ─────────────────────────────────────────────────────────────────────────────

Shape3DParams Viewer3D::selectedShape() const
{
    if (m_selectedIndex >= 0 && m_selectedIndex < m_shapes.size())
        return m_shapes[m_selectedIndex];
    return {};
}

// ─────────────────────────────────────────────────────────────────────────────
// High-level mutating API (through undo stack)
// ─────────────────────────────────────────────────────────────────────────────

void Viewer3D::addShape(const Shape3DParams &params)
{
    m_undoStack->push(new AddShape3DCommand(this, params));
}

void Viewer3D::removeSelected()
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_shapes.size()) return;
    m_undoStack->push(new RemoveShape3DCommand(this, m_selectedIndex));
}

void Viewer3D::editSelected(const Shape3DParams &newParams)
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_shapes.size()) return;
    m_undoStack->push(new EditShape3DCommand(
        this, m_selectedIndex, m_shapes[m_selectedIndex], newParams));
}

// ─────────────────────────────────────────────────────────────────────────────
// Low-level API used by commands
// ─────────────────────────────────────────────────────────────────────────────

void Viewer3D::cmdInsert(int index, const Shape3DParams &params)
{
    m_shapes.insert(index, params);
    if (m_selectedIndex >= index) ++m_selectedIndex;
    rebuildShapes();
}

void Viewer3D::cmdRemove(int index)
{
    m_shapes.removeAt(index);
    if (m_selectedIndex == index)       m_selectedIndex = -1;
    else if (m_selectedIndex > index)   --m_selectedIndex;
    rebuildShapes();
}

void Viewer3D::cmdReplace(int index, const Shape3DParams &params)
{
    m_shapes[index] = params;
    rebuildShapes();
}
