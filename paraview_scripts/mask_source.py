print("a")


ALL =                  0b0
BARYON =               0b10
BARYON_STAR =          0b100000
BARYON_WIND =          0b1000000
BARYON_STAR_FORMING =  0b10000000
DARK_AGN =             0b100000000

SELECTOR = ALL

table = vtk.vtkTable()

value = vtk.vtkVariantCreate(SELECTOR, vtk.VTK_UNSIGNED_SHORT)

arr = vtk.vtkUnsignedShortArray()
arr.SetComponent(0, 0, value)

table.GetRowData().AddArray(arr)